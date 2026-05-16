#ifndef AP_MANAGER_H
#define AP_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

class APManager {
public:
    APManager();
    void begin(const String& ssid, const String& password, int channel);
    void setSSID(const String& ssid);
    void setPassword(const String& password);
    void restartAP();
    String getSSID();
    int getClientCount();
    String getClientIP();
    bool isRunning();

private:
    String ssid;
    String password;
    int channel;
    bool running;
};

#endif
