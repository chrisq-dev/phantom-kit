#ifndef PROBE_SNIFFER_H
#define PROBE_SNIFFER_H

#include <Arduino.h>

struct ProbeDevice {
    String mac;
    String ssids[10];
    int ssidCount;
    int rssi;
    unsigned long lastSeen;
};

class ProbeSnifferModule {
public:
    ProbeSnifferModule();
    void begin();
    void startSniffing(int channel);
    void stopSniffing();
    void update();
    bool isRunning();
    String getDevicesJSON();
    int getDeviceCount();
    unsigned long getProbesCaptured();

private:
    bool running;
    int channel;
    unsigned long probesCaptured;
    ProbeDevice devices[50];
    int deviceCount;
    unsigned long lastChannelHop;
    
    void packetHandler(uint8_t* buf, uint16_t len);
    void parseProbeRequest(uint8_t* buf, uint16_t len);
    int findDevice(const String& mac);
    void addDevice(const String& mac, const String& ssid, int rssi);
};

#endif
