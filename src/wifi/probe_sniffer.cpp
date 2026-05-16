#include "probe_sniffer.h"

extern "C" {
    #include "user_interface.h"
}

extern String logBuffer;
extern void addLog(const String& msg);

ProbeSnifferModule::ProbeSnifferModule() {
    running = false;
    channel = 1;
    probesCaptured = 0;
    deviceCount = 0;
    lastChannelHop = 0;
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
    
    wifi_set_channel(channel);
    wifi_set_promiscuous_rx_cb([](uint8_t* buf, uint16_t len) {
        // This would need to be handled differently in ESP8266
    });
    
    addLog("[PROBE] Sniffing iniciado en canal " + String(channel));
}

void ProbeSnifferModule::stopSniffing() {
    running = false;
    wifi_set_promiscuous_rx_cb(nullptr);
    addLog("[PROBE] Sniffing detenido. Probes capturados: " + String(probesCaptured));
}

void ProbeSnifferModule::update() {
    if (!running) return;
    
    // Channel hopping para capturar probes en todos los canales
    unsigned long now = millis();
    if (now - lastChannelHop >= 500) { // Cambiar canal cada 500ms
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
        json += "\"rssi\":" + String(devices[i].rssi) + ",";
        json += "\"ssids\":[";
        for (int j = 0; j < devices[i].ssidCount && j < 10; j++) {
            if (j > 0) json += ",";
            json += "\"" + devices[i].ssids[j] + "\"";
        }
        json += "]}";
    }
    json += "]";
    return json;
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
        // Update existing device
        if (devices[idx].ssidCount < 10) {
            // Check if SSID already exists
            for (int i = 0; i < devices[idx].ssidCount; i++) {
                if (devices[idx].ssids[i] == ssid) return;
            }
            devices[idx].ssids[devices[idx].ssidCount++] = ssid;
        }
        devices[idx].rssi = rssi;
        devices[idx].lastSeen = millis();
    } else if (deviceCount < 50) {
        // Add new device
        devices[deviceCount].mac = mac;
        devices[deviceCount].ssids[0] = ssid;
        devices[deviceCount].ssidCount = 1;
        devices[deviceCount].rssi = rssi;
        devices[deviceCount].lastSeen = millis();
        deviceCount++;
    }
}

void ProbeSnifferModule::parseProbeRequest(uint8_t* buf, uint16_t len) {
    // Parse probe request frame to extract SSID and MAC
    // This is a simplified version
    probesCaptured++;
}

void ProbeSnifferModule::packetHandler(uint8_t* buf, uint16_t len) {
    // Check if this is a probe request
    if (len < 24) return;
    
    // Frame control check for probe request (0x40)
    if ((buf[0] & 0xFC) != 0x40) return;
    
    parseProbeRequest(buf, len);
}
