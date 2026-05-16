#ifndef CAPTIVE_PORTAL_H
#define CAPTIVE_PORTAL_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "credential_store.h"

class CaptivePortal {
public:
    CaptivePortal(CredentialStore& store);
    void begin(ESP8266WebServer& server);
    void setActive(bool active);
    bool isActive();
    void setTemplate(int index);
    int getCurrentTemplate();

private:
    CredentialStore& store;
    ESP8266WebServer* server;
    bool active;
    int currentTemplate;
    
    void handleRoot();
    void handleLogin();
    String loadTemplate(int index);
};

#endif
