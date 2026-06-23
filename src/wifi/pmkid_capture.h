#ifndef PMKID_CAPTURE_H
#define PMKID_CAPTURE_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
// PMKIDCaptureModule
//
// Captures WPA2 PMKID from the first EAPOL Key frame of a 4-way handshake.
// The PMKID attack (Jens Steube, 2018) extracts the PMKID without requiring
// a client to complete the full handshake — one EAPOL frame is enough.
//
// PMKID = HMAC-SHA1-128(PMK, "PMK Name" || AP_MAC || STA_MAC)
//
// Captured PMKIDs are stored in hashcat 22000 format:
//   WPA*02*<pmkid>*<ap_mac>*<sta_mac>*<ssid_hex>***
//
// Usage: start after deauth to force clients to re-associate and emit EAPOL.
// ---------------------------------------------------------------------------

#define MAX_PMKIDS 5

struct PMKIDEntry {
    uint8_t  pmkid[16];
    uint8_t  apMAC[6];
    uint8_t  staMAC[6];
    String   ssid;          // from associated probe sniffer data, best-effort
    char     hashcatLine[160];
    bool     valid;
    unsigned long capturedAt;
};

class PMKIDCaptureModule {
public:
    PMKIDCaptureModule();

    void begin();
    void startCapture(const String& targetBSSID);
    void stopCapture();
    bool isCapturing() const;

    int  getCaptureCount() const;
    String getHashcatOutput() const;   // All captured PMKIDs as hashcat 22000
    String getJSON() const;

private:
    bool      capturing;
    int       captureCount;
    PMKIDEntry entries[MAX_PMKIDS];
    uint8_t   targetBSSID[6];
    bool      hasTarget;

    static PMKIDCaptureModule* _instance;

    void parseBSSID(const String& s, uint8_t* out);

    // Called by static promiscuous callback
    void processFrame(const uint8_t* buf, uint16_t len);

    // Static callback registered with the SDK
    static void IRAM_ATTR pmkidPromiscCb(uint8_t* buf, uint16_t len);

    void buildHashcatLine(PMKIDEntry& e);
};

#endif
