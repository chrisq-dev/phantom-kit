#ifndef EVIL_TWIN_H
#define EVIL_TWIN_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

struct EvilTwinTarget {
    String ssid;
    String bssid;
    int channel;
    int rssi;
    bool cloned;
};

class EvilTwinModule {
public:
    EvilTwinModule();
    void begin();
    void scanTargets();
    void scanAllChannels();
    bool cloneTarget(const String& ssid, const String& bssid, int channel);
    void stopClone();
    void update();
    bool isActive();
    String getTargetsJSON();
    int getTargetCount();
    String getActiveTarget();

private:
    bool active;
    String activeSSID;
    String activeBSSID;
    int activeChannel;
    EvilTwinTarget targets[20];
    int targetCount;
    
    void generateSimilarBSSID(const String& original, uint8_t* newBssid);
};

#endif
