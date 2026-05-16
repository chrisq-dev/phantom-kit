#ifndef BEACON_FLOOD_H
#define BEACON_FLOOD_H

#include <Arduino.h>

class BeaconFloodModule {
public:
    BeaconFloodModule();
    void begin();
    void startFlood(int channel);
    void stopFlood();
    void update();
    bool isRunning();
    unsigned long getBeaconsSent();
    void setCustomSSIDs(const String ssids[], int count);
    int getSSIDCount();

private:
    bool running;
    int channel;
    unsigned long beaconsSent;
    unsigned long lastBeacon;
    String customSSIDs[50];
    int customSSIDCount;
    
    void sendBeacon(const String& ssid, const uint8_t* bssid);
    void generateRandomBSSID(uint8_t* bssid);
    String getRandomSSID();
};

#endif
