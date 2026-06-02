#include "ap_manager.h"
#include "config.h"

extern "C" {
    #include "user_interface.h"
}

APManager::APManager() {
    ssid = AP_SSID;
    password = AP_PASSWORD;
    channel = AP_CHANNEL;
    running = false;
    stealthMode = STEALTH_MODE;
}

void APManager::begin(const String& ssid, const String& password, int channel) {
    this->ssid = ssid;
    this->password = password;
    this->channel = channel;
    
    WiFi.softAPConfig(
        IPAddress(AP_IP_ADDRESS),
        IPAddress(AP_IP_ADDRESS),
        IPAddress(AP_SUBNET)
    );
    
    WiFi.softAP(ssid.c_str(), password.c_str(), channel);
    running = true;

    // Apply stealth mode via SDK (hide SSID from beacon frames)
    if (stealthMode) {
        struct softap_config conf;
        wifi_softap_get_config(&conf);
        conf.ssid_hidden = 1;
        wifi_softap_set_config(&conf);
    }

    Serial.printf("[AP] SSID: %s%s\n", ssid.c_str(), stealthMode ? " (hidden)" : "");
    Serial.printf("[AP] IP: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.printf("[AP] Canal: %d\n", channel);
}

void APManager::setSSID(const String& newSSID) {
    ssid = newSSID;
}

void APManager::setPassword(const String& newPassword) {
    password = newPassword;
}

void APManager::restartAP() {
    WiFi.softAPdisconnect(true);
    delay(500);
    begin(ssid, password, channel);
}

String APManager::getSSID() {
    return ssid;
}

int APManager::getClientCount() {
    return WiFi.softAPgetStationNum();
}

String APManager::getClientIP() {
    return WiFi.softAPIP().toString();
}

bool APManager::isRunning() {
    return running;
}

void APManager::setStealthMode(bool enabled) {
    stealthMode = enabled;
    if (running) {
        struct softap_config conf;
        wifi_softap_get_config(&conf);
        conf.ssid_hidden = enabled ? 1 : 0;
        wifi_softap_set_config(&conf);
        Serial.printf("[AP] Stealth mode: %s\n", enabled ? "ON" : "OFF");
    }
}

bool APManager::isStealthMode() {
    return stealthMode;
}
