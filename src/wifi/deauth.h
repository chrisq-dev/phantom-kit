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

// Max number of scanned APs/client MACs retained in RAM.
#define MAX_DEAUTH_TARGETS 12
#define MAX_DEAUTH_CLIENTS 10

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
    int getClientCount();
    void scanTargets();
    void scanAllChannels();
    void scanClients(uint16_t windowMs = 800);
    String getActiveTarget();
    int getTargetChannel();

private:
    bool running;
    String targetBSSID;
    int targetChannel;
    unsigned long framesSent;
    unsigned long lastScan;
    DeauthTarget targets[MAX_DEAUTH_TARGETS];
    int targetCount;
    String activeTarget;

    // Unicast client list
    uint8_t clientMACs[MAX_DEAUTH_CLIENTS][6];
    int clientCount;

    void sendDeauthFrame(const uint8_t* bssid, const uint8_t* clientMac);
    bool parseBSSID(const String& bssidStr, uint8_t* out);
    void parseScanResult();
};

#endif
