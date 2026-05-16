#ifndef DEAUTH_H
#define DEAUTH_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

struct DeauthTarget {
    String ssid;
    String bssid;
    int channel;
    int rssi;
    int clientCount;
};

class DeauthModule {
public:
    DeauthModule();
    void begin();
    void startAttack(const String& bssid, int channel);
    void stopAttack();
    void update();
    bool isRunning();
    String getTargetsJSON();
    int getTargetCount();
    unsigned long getFramesSent();
    void scanTargets();
    void scanAllChannels();
    String getActiveTarget();
    int getTargetChannel();

private:
    bool running;
    String targetBSSID;
    int targetChannel;
    unsigned long framesSent;
    unsigned long lastScan;
    DeauthTarget targets[20];
    int targetCount;
    String activeTarget;
    
    void sendDeauthFrame(const uint8_t* bssid, const uint8_t* clientMac);
    void parseScanResult();
};

#endif
