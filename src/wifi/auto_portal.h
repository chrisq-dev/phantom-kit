#ifndef AUTO_PORTAL_H
#define AUTO_PORTAL_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

class AutoPortalModule {
public:
    AutoPortalModule();
    void begin();
    void startAutoScan();
    void stopAutoScan();
    bool isScanning();
    String getMostPopularSSID();
    int getMostPopularClientCount();
    String getScanResultsJSON();
    void analyzeResults();

private:
    bool scanning;
    unsigned long lastScan;
    String popularSSID;
    int popularClientCount;
    
    struct NetworkInfo {
        String ssid;
        int clientCount;
        int rssi;
    };
    NetworkInfo networks[20];
    int networkCount;
    
    void scanNetworks();
};

#endif
