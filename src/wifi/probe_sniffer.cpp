#include "probe_sniffer.h"
#include "oui.h"
#include "auto_portal.h"

extern "C" {
    #include "user_interface.h"
}

extern String logBuffer;
extern void addLog(const String& msg);

// ---------------------------------------------------------------------------
// ESP8266 promiscuous mode frame structure
// The SDK prepends a 36-byte RxControl header before the raw 802.11 frame.
//
// 802.11 Probe Request layout (after the 36-byte RxControl):
//   [0..1]   Frame Control  (0x40 0x00 = Probe Request)
//   [2..3]   Duration
//   [4..9]   DA  = FF:FF:FF:FF:FF:FF (broadcast)
//   [10..15] SA  = source MAC (the probing device)
//   [16..21] BSSID = FF:FF:FF:FF:FF:FF
//   [22..23] Sequence Control
//   [24+]    Tagged parameters
//              Tag 0x00 = SSID: [0x00][len][ssid bytes]
// ---------------------------------------------------------------------------

#define RX_CTRL_LEN 36

// Static singleton pointer so the ICACHE_RAM_ATTR callback can reach the module
static ProbeSnifferModule* _probeInstance = nullptr;

static void IRAM_ATTR probePromiscCb(uint8_t* buf, uint16_t len) {
    if (!_probeInstance) return;

    // Minimum frame: RxControl (36) + FC (2) + Dur (2) + DA (6) + SA (6) + BSSID (6) + SeqCtrl (2) = 60 bytes
    if (len < 60) return;

    // Frame Control byte 0: 0x40 = Probe Request subtype
    uint8_t fc0 = buf[RX_CTRL_LEN + 0];
    uint8_t fc1 = buf[RX_CTRL_LEN + 1];
    if (fc0 != 0x40 || fc1 != 0x00) return;

    // Extract RSSI from RxControl (first signed byte)
    int8_t rssi = (int8_t)buf[0];

    // Extract Source MAC (SA) at offset 36 + 10 = 46
    char mac[18];
    snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
             buf[46], buf[47], buf[48], buf[49], buf[50], buf[51]);

    // Parse SSID tagged parameter starting at offset 36 + 24 = 60
    // Tag 0x00 is SSID
    int pos = RX_CTRL_LEN + 24;
    String ssid = "";

    while (pos + 1 < (int)len) {
        uint8_t tag = buf[pos];
        uint8_t tagLen = buf[pos + 1];

        if (pos + 2 + tagLen > (int)len) break;

        if (tag == 0x00) {
            // SSID element
            if (tagLen == 0) {
                ssid = "[broadcast]";
            } else {
                for (int i = 0; i < tagLen && i < 32; i++) {
                    char c = (char)buf[pos + 2 + i];
                    if (c >= 0x20 && c < 0x7F) ssid += c;
                    else ssid += '.';
                }
            }
            break;
        }
        pos += 2 + tagLen;
    }

    if (ssid.length() == 0) return;  // skip malformed frames

    _probeInstance->addDevice(String(mac), ssid, (int)rssi);
}

// ---------------------------------------------------------------------------

ProbeSnifferModule::ProbeSnifferModule() {
    running = false;
    channel = 1;
    probesCaptured = 0;
    deviceCount = 0;
    lastChannelHop = 0;
    karmaEnabled = false;
    karmaCallback = nullptr;
}

void ProbeSnifferModule::begin() {
    addLog("[PROBE] Modulo iniciado");
}

void ProbeSnifferModule::startSniffing(int ch) {
    if (running) return;

    channel = ch;
    running = true;
    probesCaptured = 0;
    deviceCount = 0;

    _probeInstance = this;

    wifi_set_channel(channel);
    wifi_set_promiscuous_rx_cb(probePromiscCb);
    wifi_promiscuous_enable(1);

    addLog("[PROBE] Sniffing iniciado en canal " + String(channel));
}

void ProbeSnifferModule::stopSniffing() {
    running = false;
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(nullptr);
    _probeInstance = nullptr;
    addLog("[PROBE] Sniffing detenido. Probes capturados: " + String(probesCaptured)
           + " Dispositivos: " + String(deviceCount));
}

void ProbeSnifferModule::update() {
    if (!running) return;

    // Channel hop every 500ms to capture probes on all channels
    unsigned long now = millis();
    if (now - lastChannelHop >= 500) {
        channel++;
        if (channel > 13) channel = 1;
        wifi_set_channel(channel);
        lastChannelHop = now;
    }
}

bool ProbeSnifferModule::isRunning() {
    return running;
}

unsigned long ProbeSnifferModule::getProbesCaptured() {
    return probesCaptured;
}

int ProbeSnifferModule::getDeviceCount() {
    return deviceCount;
}

String ProbeSnifferModule::getDevicesJSON() {
    String json = "[";
    for (int i = 0; i < deviceCount; i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"mac\":\"" + devices[i].mac + "\",";
        json += "\"vendor\":\"" + devices[i].vendor + "\",";
        json += "\"rssi\":" + String(devices[i].rssi) + ",";
        json += "\"ssids\":[";
        for (int j = 0; j < devices[i].ssidCount && j < MAX_PROBE_SSIDS_PER_DEVICE; j++) {
            if (j > 0) json += ",";
            json += "\"" + devices[i].ssids[j] + "\"";
        }
        json += "]}";
    }
    json += "]";
    return json;
}

void ProbeSnifferModule::setKarmaMode(bool enabled, KarmaCallback cb) {
    karmaEnabled = enabled;
    karmaCallback = cb;
    if (enabled) {
        addLog("[KARMA] Modo Karma ACTIVO - responde a cualquier probe request");
    } else {
        addLog("[KARMA] Modo Karma desactivado");
        karmaActiveSSID = "";
    }
}

bool ProbeSnifferModule::isKarmaActive() const {
    return karmaEnabled;
}

String ProbeSnifferModule::getKarmaSSID() const {
    return karmaActiveSSID;
}

int ProbeSnifferModule::findDevice(const String& mac) {
    for (int i = 0; i < deviceCount; i++) {
        if (devices[i].mac == mac) return i;
    }
    return -1;
}

void ProbeSnifferModule::addDevice(const String& mac, const String& ssid, int rssi) {
    int idx = findDevice(mac);
    if (idx >= 0) {
        // Update existing device: add SSID if not already seen
        if (devices[idx].ssidCount < MAX_PROBE_SSIDS_PER_DEVICE) {
            for (int i = 0; i < devices[idx].ssidCount; i++) {
                if (devices[idx].ssids[i] == ssid) {
                    // Already known SSID — just update rssi and count
                    devices[idx].rssi = rssi;
                    devices[idx].lastSeen = millis();
                    probesCaptured++;
                    return;
                }
            }
            devices[idx].ssids[devices[idx].ssidCount++] = ssid;
        }
        devices[idx].rssi = rssi;
        devices[idx].lastSeen = millis();
    } else if (deviceCount < MAX_PROBE_DEVICES) {
        // New device — OUI lookup
        uint8_t macBytes[3];
        sscanf(mac.c_str(), "%hhx:%hhx:%hhx", &macBytes[0], &macBytes[1], &macBytes[2]);

        devices[deviceCount].mac         = mac;
        devices[deviceCount].vendor      = lookupOUI(macBytes);
        devices[deviceCount].ssids[0]    = ssid;
        devices[deviceCount].ssidCount   = 1;
        devices[deviceCount].rssi        = rssi;
        devices[deviceCount].lastSeen    = millis();
        deviceCount++;
        addLog("[PROBE] Nuevo: " + mac + " (" + devices[deviceCount - 1].vendor + ") -> " + ssid);

        // Karma: trigger callback for first probe of this SSID from any device
        if (karmaEnabled && karmaCallback && ssid != "[broadcast]") {
            int tpl = AutoPortalModule::suggestTemplate(ssid);
            karmaCallback(ssid, tpl);
        }
    }
    probesCaptured++;
}

void ProbeSnifferModule::parseProbeRequest(uint8_t* buf, uint16_t len) {
    // Parsing is handled directly in the promiscuous callback
    (void)buf; (void)len;
}

void ProbeSnifferModule::packetHandler(uint8_t* buf, uint16_t len) {
    // Handled via static callback
    (void)buf; (void)len;
}
