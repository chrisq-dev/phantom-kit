#include "ap_manager.h"
#include "config.h"

APManager::APManager() {
    ssid = AP_SSID;
    password = AP_PASSWORD;
    channel = AP_CHANNEL;
    running = false;
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
    
    Serial.printf("[AP] SSID: %s\n", ssid.c_str());
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
