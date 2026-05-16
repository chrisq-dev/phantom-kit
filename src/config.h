#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// WiFi AP Configuration
#define AP_SSID "PhantomKit"
#define AP_PASSWORD "phantom123"
#define AP_CHANNEL 6  // Canal más común para mejor compatibilidad
#define AP_MAX_CLIENTS 10
#define AP_IP_ADDRESS 192, 168, 4, 1
#define AP_SUBNET 255, 255, 255, 0

// Channel Hopping Configuration
#define CHANNEL_HOPPING_ENABLED true
#define CHANNEL_HOP_INTERVAL 500  // ms por canal
#define MIN_CHANNEL 1
#define MAX_CHANNEL 13

// Dashboard
#define DASHBOARD_PORT 80
#define MAX_CREDENTIALS 50
#define MAX_LOGS 30

// Templates
#define TEMPLATE_COUNT 8
extern const char* TEMPLATES[TEMPLATE_COUNT];
extern const char* TEMPLATE_NAMES[TEMPLATE_COUNT];

// Modules State
extern bool portalActive;
extern int currentTemplate;
extern String currentSSID;
extern String logBuffer;

#endif
