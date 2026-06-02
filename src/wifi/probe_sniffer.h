#ifndef PROBE_SNIFFER_H
#define PROBE_SNIFFER_H

#include <Arduino.h>

// Karma callback: called when a new unique SSID probe is detected
// Parameters: ssid, suggestedTemplateIndex
typedef void (*KarmaCallback)(const String& ssid, int templateIdx);

struct ProbeDevice {
    String mac;
    String vendor;       // OUI lookup result
    String ssids[10];
    int    ssidCount;
    int    rssi;
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
    void addDevice(const String& mac, const String& ssid, int rssi);  // called by promiscuous callback

    // Karma Attack mode
    void setKarmaMode(bool enabled, KarmaCallback cb = nullptr);
    bool isKarmaActive() const;
    String getKarmaSSID() const;   // Current SSID being spoofed by karma

private:
    bool running;
    int channel;
    unsigned long probesCaptured;
    ProbeDevice devices[50];
    int deviceCount;
    unsigned long lastChannelHop;

    // Karma
    bool karmaEnabled;
    KarmaCallback karmaCallback;
    String karmaActiveSSID;

    void packetHandler(uint8_t* buf, uint16_t len);
    void parseProbeRequest(uint8_t* buf, uint16_t len);
    int findDevice(const String& mac);
};

#endif

