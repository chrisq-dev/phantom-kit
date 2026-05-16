#include "evil_twin.h"
#include "config.h"

extern "C" {
    #include "user_interface.h"
}

extern String logBuffer;
extern void addLog(const String& msg);

EvilTwinModule::EvilTwinModule() {
    active = false;
    targetCount = 0;
}

void EvilTwinModule::begin() {
    addLog("[EVIL_TWIN] Modulo iniciado");
}

void EvilTwinModule::scanTargets() {
    int n = WiFi.scanNetworks();
    targetCount = 0;
    
    for (int i = 0; i < n && targetCount < 20; i++) {
        targets[targetCount].ssid = WiFi.SSID(i);
        targets[targetCount].bssid = WiFi.BSSIDstr(i);
        targets[targetCount].channel = WiFi.channel(i);
        targets[targetCount].rssi = WiFi.RSSI(i);
        targets[targetCount].cloned = false;
        targetCount++;
    }
    
    WiFi.scanDelete();
    addLog("[EVIL_TWIN] Escaneo completado: " + String(targetCount) + " redes");
}

void EvilTwinModule::scanAllChannels() {
    targetCount = 0;
    
    for (int ch = 1; ch <= 13; ch++) {
        wifi_set_channel(ch);
        delay(100);
        
        int n = WiFi.scanNetworks();
        for (int i = 0; i < n && targetCount < 20; i++) {
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
                targets[targetCount].cloned = false;
                targetCount++;
            }
        }
        WiFi.scanDelete();
    }
    
    wifi_set_channel(AP_CHANNEL);
    addLog("[EVIL_TWIN] Escaneo multi-canal completado: " + String(targetCount) + " redes");
}

bool EvilTwinModule::cloneTarget(const String& ssid, const String& bssid, int channel) {
    if (active) return false;
    
    activeSSID = ssid;
    activeBSSID = bssid;
    activeChannel = channel;
    active = true;
    
    addLog("[EVIL_TWIN] Clonando: " + ssid + " (" + bssid + ") canal " + String(channel));
    return true;
}

void EvilTwinModule::stopClone() {
    active = false;
    activeSSID = "";
    activeBSSID = "";
    addLog("[EVIL_TWIN] Clon detenido");
}

void EvilTwinModule::update() {
    // Evil twin operates by creating a duplicate AP
    // No periodic update needed in this implementation
}

bool EvilTwinModule::isActive() {
    return active;
}

String EvilTwinModule::getActiveTarget() {
    return activeSSID;
}

int EvilTwinModule::getTargetCount() {
    return targetCount;
}

String EvilTwinModule::getTargetsJSON() {
    String json = "[";
    for (int i = 0; i < targetCount; i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + targets[i].ssid + "\",";
        json += "\"bssid\":\"" + targets[i].bssid + "\",";
        json += "\"channel\":" + String(targets[i].channel) + ",";
        json += "\"rssi\":" + String(targets[i].rssi);
        json += "}";
    }
    json += "]";
    return json;
}

void EvilTwinModule::generateSimilarBSSID(const String& original, uint8_t* newBssid) {
    // Generate a BSSID that's similar but not identical
    // This is a simplified version
    for (int i = 0; i < 6; i++) {
        newBssid[i] = random(256);
    }
    newBssid[0] = 0x02; // Locally administered
}
