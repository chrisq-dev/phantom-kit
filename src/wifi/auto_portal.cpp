#include "auto_portal.h"

extern String logBuffer;
extern void addLog(const String& msg);

AutoPortalModule::AutoPortalModule() {
    scanning = false;
    lastScan = 0;
    popularClientCount = 0;
    networkCount = 0;
}

void AutoPortalModule::begin() {
    addLog("[AUTO_PORTAL] Modulo iniciado");
}

void AutoPortalModule::startAutoScan() {
    scanning = true;
    addLog("[AUTO_PORTAL] Escaneo automatico iniciado");
}

void AutoPortalModule::stopAutoScan() {
    scanning = false;
    addLog("[AUTO_PORTAL] Escaneo automatico detenido");
}

bool AutoPortalModule::isScanning() {
    return scanning;
}

String AutoPortalModule::getMostPopularSSID() {
    return popularSSID;
}

int AutoPortalModule::getMostPopularClientCount() {
    return popularClientCount;
}

String AutoPortalModule::getScanResultsJSON() {
    String json = "[";
    for (int i = 0; i < networkCount; i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + networks[i].ssid + "\",";
        json += "\"clients\":" + String(networks[i].clientCount) + ",";
        json += "\"rssi\":" + String(networks[i].rssi);
        json += "}";
    }
    json += "]";
    return json;
}

void AutoPortalModule::scanNetworks() {
    int n = WiFi.scanNetworks();
    networkCount = 0;
    
    for (int i = 0; i < n && networkCount < 20; i++) {
        networks[networkCount].ssid = WiFi.SSID(i);
        networks[networkCount].clientCount = 0; // Would need packet analysis
        networks[networkCount].rssi = WiFi.RSSI(i);
        networkCount++;
    }
    
    WiFi.scanDelete();
}

void AutoPortalModule::analyzeResults() {
    if (millis() - lastScan < 10000) return;
    lastScan = millis();
    
    scanNetworks();
    
    // Find the network with best signal (most likely to be the target)
    int bestRSSI = -100;
    for (int i = 0; i < networkCount; i++) {
        if (networks[i].rssi > bestRSSI && networks[i].ssid.length() > 0) {
            bestRSSI = networks[i].rssi;
            popularSSID = networks[i].ssid;
            popularClientCount = networks[i].clientCount;
        }
    }
    
    if (popularSSID.length() > 0) {
        addLog("[AUTO_PORTAL] Red mas popular: " + popularSSID + " (" + String(bestRSSI) + "dBm)");
    }
}
