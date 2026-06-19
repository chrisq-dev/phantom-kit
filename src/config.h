#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
// WiFi AP Configuration (portal management network)
// ---------------------------------------------------------------------------
#define AP_SSID        "PhantomKit"
#define AP_PASSWORD    "change-me-phantomkit"
#define AP_CHANNEL     6
#define AP_MAX_CLIENTS 10
#define AP_IP_ADDRESS  192, 168, 4, 1
#define AP_SUBNET      255, 255, 255, 0

// ---------------------------------------------------------------------------
// Stealth Mode
// When true, the management AP SSID is hidden from Wi-Fi scans.
// You must know the SSID to connect (set it manually on your device).
// ---------------------------------------------------------------------------
#define STEALTH_MODE false

// ---------------------------------------------------------------------------
// Dashboard Authentication
// Separate from the AP password. Required to access /dashboard.
// ---------------------------------------------------------------------------
#define DASHBOARD_PASSWORD "change-me-auditor"

// ---------------------------------------------------------------------------
// Portfolio / demo safety
// Redacts captured fields in dashboard/API responses by default. Disable only
// inside an authorized lab when you explicitly need raw collected values.
// ---------------------------------------------------------------------------
#define DASHBOARD_REDACT_CREDENTIALS true

// ---------------------------------------------------------------------------
// Emergency Wipe
// GPIO0 is the FLASH button on NodeMCU. Hold for GPIO_WIPE_HOLD_MS to wipe.
// Set GPIO_WIPE_PIN to -1 to disable hardware wipe button.
// ---------------------------------------------------------------------------
#define GPIO_WIPE_PIN      0        // NodeMCU FLASH button
#define GPIO_WIPE_HOLD_MS  3000     // Hold 3 seconds to wipe

// ---------------------------------------------------------------------------
// Karma Attack
// When probe sniffer is active with karma enabled, the AP SSID will change
// to match the first non-broadcast probe request detected.
// ---------------------------------------------------------------------------
#define KARMA_MODE_DEFAULT false    // Disabled by default, enable from dashboard

// ---------------------------------------------------------------------------
// Uplink WiFi (optional)
// If set, the ESP8266 connects to this network in AP+STA mode to enable:
//   - NTP real timestamps
//   - Webhook notifications
// Leave both empty ("") to disable uplink.
// ---------------------------------------------------------------------------
#define UPLINK_SSID     ""
#define UPLINK_PASSWORD ""

// ---------------------------------------------------------------------------
// NTP / Timezone
// UTC offset in seconds. UTC-6 (Mexico Centro) = -21600
// ---------------------------------------------------------------------------
#define NTP_SERVER    "pool.ntp.org"
#define NTP_OFFSET    -21600   // seconds (UTC-6)
#define NTP_DST       0

// ---------------------------------------------------------------------------
// Channel Hopping
// ---------------------------------------------------------------------------
#define CHANNEL_HOPPING_ENABLED true
#define CHANNEL_HOP_INTERVAL    500
#define MIN_CHANNEL             1
#define MAX_CHANNEL             13

// ---------------------------------------------------------------------------
// Dashboard / Storage limits
// ---------------------------------------------------------------------------
#define DASHBOARD_PORT   80
#define MAX_CREDENTIALS  50
#define MAX_LOGS         30

// ---------------------------------------------------------------------------
// Templates
// ---------------------------------------------------------------------------
#define TEMPLATE_COUNT 8
extern const char* TEMPLATES[TEMPLATE_COUNT];
extern const char* TEMPLATE_NAMES[TEMPLATE_COUNT];

// ---------------------------------------------------------------------------
// Global state (defined in main.cpp)
// ---------------------------------------------------------------------------
extern bool   portalActive;
extern int    currentTemplate;
extern String currentSSID;
extern String logBuffer;

#endif
