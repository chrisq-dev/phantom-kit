#include "beacon_flood.h"

extern "C" {
    #include "user_interface.h"
}

extern String logBuffer;
extern void addLog(const String& msg);

// Default SSID list for beacon flood
const char* defaultSSIDs[] = {
    "WiFi Gratis", "Free Airport WiFi", "Starbucks_WiFi", "Hotel_Guest",
    "Marriott_WiFi", "Hilton_Honors", "NETGEAR47", "HP-Print-2B",
    "DIRECT-roku-428", "AndroidAP", "iPhone de Juan", "Mi WiFi 2.4G",
    "TP-Link_A420", "ASUS_2G", "Linksys04562", "Belkin.456",
    "ATTWiFi", "XFINITY", "SpectrumWiFi", "CoxWiFi",
    "Free Public WiFi", "Virus Distribution Center", "FBI Surveillance Van",
    "Pretty Fly for a WiFi", "Bill Wi the Science Fi", "LAN Solo",
    "The LAN Before Time", "Silence of the LANs", "Drop it like it's Hotspot",
    "Martin Router King", "Wi-Fight the Feeling", "Tell My WiFi Love Her",
    "No More Mr. WiFi", "Wu-Tang LAN", "Obi-LAN Kenobi", "The Password is 1234",
    "Connect for Identity Theft", "Virus.exe", "Malware Distribution",
    "Click Here for Prizes", "Free iPhone Giveaway", "Bitcoin Miner",
    "Area 51 Test Site", "Skynet Global Defense", "Umbrella Corporation",
    "Cyberdyne Systems", "Stark Industries", "Wayne Enterprises",
    "Oscorp Industries", "Aperture Science", "Black Mesa Research"
};
const int defaultSSIDCount = sizeof(defaultSSIDs) / sizeof(defaultSSIDs[0]);

BeaconFloodModule::BeaconFloodModule() {
    running = false;
    channel = 1;
    beaconsSent = 0;
    lastBeacon = 0;
    customSSIDCount = 0;
}

void BeaconFloodModule::begin() {
    addLog("[BEACON] Modulo iniciado con " + String(defaultSSIDCount) + " SSIDs");
}

void BeaconFloodModule::startFlood(int ch) {
    if (running) return;
    
    channel = ch;
    running = true;
    beaconsSent = 0;
    
    wifi_set_channel(channel);
    
    addLog("[BEACON] Flood iniciado en canal " + String(channel));
}

void BeaconFloodModule::stopFlood() {
    running = false;
    addLog("[BEACON] Flood detenido. Beacons enviados: " + String(beaconsSent));
}

void BeaconFloodModule::update() {
    if (!running) return;
    
    unsigned long now = millis();
    if (now - lastBeacon >= 100) { // Enviar beacon cada 100ms
        uint8_t bssid[6];
        generateRandomBSSID(bssid);
        String ssid = getRandomSSID();
        sendBeacon(ssid, bssid);
        lastBeacon = now;
    }
}

bool BeaconFloodModule::isRunning() {
    return running;
}

unsigned long BeaconFloodModule::getBeaconsSent() {
    return beaconsSent;
}

int BeaconFloodModule::getSSIDCount() {
    return customSSIDCount > 0 ? customSSIDCount : defaultSSIDCount;
}

void BeaconFloodModule::setCustomSSIDs(const String ssids[], int count) {
    customSSIDCount = count > 24 ? 24 : count;
    for (int i = 0; i < customSSIDCount; i++) {
        customSSIDs[i] = ssids[i];
    }
}

String BeaconFloodModule::getRandomSSID() {
    if (customSSIDCount > 0) {
        return customSSIDs[random(customSSIDCount)];
    }
    return String(defaultSSIDs[random(defaultSSIDCount)]);
}

void BeaconFloodModule::generateRandomBSSID(uint8_t* bssid) {
    bssid[0] = 0x02; // Locally administered
    for (int i = 1; i < 6; i++) {
        bssid[i] = random(256);
    }
}

void BeaconFloodModule::sendBeacon(const String& ssid, const uint8_t* bssid) {
    uint8_t ssidLen = ssid.length();
    int packetLen = 35 + ssidLen;
    uint8_t* packet = new uint8_t[packetLen];
    
    // Frame control: Beacon (0x80)
    packet[0] = 0x80;
    packet[1] = 0x00;
    
    // Duration
    packet[2] = 0x00;
    packet[3] = 0x00;
    
    // Destination (broadcast)
    memset(&packet[4], 0xFF, 6);
    
    // Source (random BSSID)
    memcpy(&packet[10], bssid, 6);
    
    // BSSID
    memcpy(&packet[16], bssid, 6);
    
    // Sequence control
    packet[22] = random(256);
    packet[23] = random(256);
    
    // Timestamp
    memset(&packet[24], 0, 8);
    
    // Beacon interval (100 TU = 102.4ms)
    packet[32] = 0x64;
    packet[33] = 0x00;
    
    // Capability info
    packet[34] = 0x31;
    packet[35] = 0x04;
    
    // SSID parameter set
    packet[36] = 0x00; // SSID tag
    packet[37] = ssidLen;
    memcpy(&packet[38], ssid.c_str(), ssidLen);
    
    // Supported rates
    int ratesPos = 38 + ssidLen;
    packet[ratesPos] = 0x01; // Supported rates tag
    packet[ratesPos + 1] = 0x08; // 8 rates
    packet[ratesPos + 2] = 0x82; // 1 Mbps
    packet[ratesPos + 3] = 0x84; // 2 Mbps
    packet[ratesPos + 4] = 0x8B; // 5.5 Mbps
    packet[ratesPos + 5] = 0x96; // 11 Mbps
    packet[ratesPos + 6] = 0x24; // 18 Mbps
    packet[ratesPos + 7] = 0x30; // 24 Mbps
    packet[ratesPos + 8] = 0x48; // 36 Mbps
    packet[ratesPos + 9] = 0x6C; // 54 Mbps
    
    // DS parameter set (channel)
    int dsPos = ratesPos + 10;
    packet[dsPos] = 0x03; // DS tag
    packet[dsPos + 1] = 0x01; // Length
    packet[dsPos + 2] = channel;
    
    // Send the frame
    wifi_send_pkt_freedom(packet, packetLen, 0);
    beaconsSent++;
    
    delete[] packet;
}
