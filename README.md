<div align="center">

[English](README.md) | [Español](README.es.md)

# ESP8266 PhantomKit

**Wi-Fi security auditing and social engineering testing suite for ESP8266**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ESP8266-orange.svg)](https://www.espressif.com/en/products/socs/esp8266)
[![Framework](https://img.shields.io/badge/Framework-Arduino%20%2F%20PlatformIO-teal.svg)](https://platformio.org/)
[![Release](https://img.shields.io/github/v/release/chrisq-dev/phantom-kit?color=brightgreen)](https://github.com/chrisq-dev/phantom-kit/releases)
[![Issues](https://img.shields.io/github/issues/chrisq-dev/phantom-kit)](https://github.com/chrisq-dev/phantom-kit/issues)

*Turn your ESP8266 into a self-contained Wi-Fi security auditing platform.*  
*No external infrastructure. No dependencies. Just the chip.*

</div>

---

## What is PhantomKit?

ESP8266 PhantomKit is an open-source Wi-Fi security auditing tool that runs entirely on an ESP8266 microcontroller (NodeMCU, Wemos D1 Mini, etc.). It implements real social engineering techniques through a web dashboard accessible from any browser-enabled device, with no laptop, server, or internet connection required.

Designed for:
- Security students who want to learn with real hardware
- Auditors conducting authorized phishing simulations
- Security awareness trainers in corporate environments

---

## Features

### Web Dashboard
- Full control from any device with a browser
- Real-time credential capture table
- Live event log with timestamps
- Configurable SSID without reflashing
- Fully autonomous — no internet or external server needed

### Attack Modules

| Module | Description | Technique |
|--------|-------------|-----------|
| Evil Portal | Captive portal with 8 cloned templates | DNS Spoofing + Captive Portal |
| Deauth Attack | Disconnects devices from a target network | IEEE 802.11 Deauth Frames |
| Beacon Flood | Saturates scan lists with 50+ fake SSIDs | Fake Beacon Frames |
| Probe Sniffer | Detects networks sought by nearby devices | Passive Probe Capture |
| Evil Twin | Clones an existing Wi-Fi network | SSID/BSSID Spoofing |
| Channel Hopping | Automatic rotation across channels 1–13 | Multi-channel Scanning |
| Auto-Portal | Suggests the best template for the environment | Environment Analysis |

### Evil Portal Templates

| Template | Style | Fields |
|----------|-------|--------|
| Facebook | Classic blue, centered card | Email + Password |
| Instagram | Purple/orange gradient | Username + Password |
| Microsoft | Fluent Design | Email + Password |
| X (Twitter) | Dark mode, black | Username + Password |
| Google | Ultra minimal | Email + Password |
| WiFi Login | Generic modern | Wi-Fi Password x2 |
| Netflix | Dark background, red | Email + Password |
| WhatsApp | Green, mobile-first | Phone + Code |

---

## Quick Start

### Requirements

- ESP8266 (NodeMCU v2, Wemos D1 Mini, or similar)
- [PlatformIO](https://platformio.org/) installed
- USB data cable
- Python 3.x

### Installation

```bash
# 1. Clone the repository
git clone https://github.com/chrisq-dev/phantom-kit.git
cd phantom-kit

# 2. Build the firmware
pio run

# 3. Upload templates to the filesystem (LittleFS)
pio run --target uploadfs

# 4. Flash the firmware
pio run --target upload

# 5. Connect to the network created by the ESP8266
#    SSID:     PhantomKit
#    Password: phantom123

# 6. Open the dashboard in your browser
#    http://192.168.4.1/dashboard
```

### Configuration

Edit `src/config.h` to change the SSID, password, or channel before compiling:

```cpp
#define AP_SSID     "PhantomKit"
#define AP_PASSWORD "phantom123"
#define AP_CHANNEL  6
```

---

## Project Structure

```
phantom-kit/
├── platformio.ini              # PlatformIO configuration
├── src/
│   ├── main.cpp                # Entry point
│   ├── config.h / config.cpp   # Global configuration
│   ├── wifi/
│   │   ├── ap_manager.*        # Access Point management
│   │   ├── dns_server.*        # DNS spoofing (captive portal)
│   │   ├── web_server.*        # Web dashboard + REST API
│   │   ├── deauth.*            # Deauth Attack module
│   │   ├── beacon_flood.*      # Beacon Flood module
│   │   ├── probe_sniffer.*     # Probe Sniffer module
│   │   ├── evil_twin.*         # Evil Twin module
│   │   ├── auto_portal.*       # Auto-Portal module
│   │   └── channel_hopper.*    # Automatic channel hopping
│   └── portal/
│       ├── captive_portal.*    # Captive portal logic
│       └── credential_store.*  # Credential storage
├── data/templates/             # HTML templates (LittleFS)
│   ├── facebook.html
│   ├── instagram.html
│   ├── microsoft.html
│   ├── x.html
│   ├── google.html
│   ├── wifi_login.html
│   ├── netflix.html
│   └── whatsapp.html
├── docs/
│   ├── setup.md                # Detailed installation guide
│   ├── usage.md                # Usage guide and scenarios
│   └── legal.md                # Legal disclaimer
├── CHANGELOG.md                # Version history
└── README.md
```

---

## Audit Scenarios

### Scenario 1 — Corporate Office
```
Goal:     Demonstrate internal phishing risk
SSID:     CorpWiFi-Guests
Template: Microsoft Login
Modules:  Evil Portal + Deauth on corporate network
```

### Scenario 2 — Hotel / Airport
```
Goal:     Demonstrate public network risk
SSID:     Free Hotel WiFi
Template: WiFi Login
Modules:  Evil Twin of venue network
```

### Scenario 3 — Security Awareness Training
```
Goal:     Employee security awareness workshop
SSID:     Free Airport WiFi
Template: Google
Modules:  Evil Portal with auditor report
```

---

## Advanced Configuration

### Channel Hopping
The ESP8266 automatically rotates across channels 1–13 to:
- Scan all networks in the area
- Capture probe requests on any channel
- Execute attacks on targets across all channels

### AP Channel
Default is channel 6 (most compatible). Configurable in `config.h`:
```cpp
#define AP_CHANNEL 6
```

### Dependencies (PlatformIO)
```ini
lib_deps =
    ESP8266WiFi
    DNSServer
    ESP8266WebServer
    bblanchon/ArduinoJson @ ^6.21.3
```

---

## Roadmap

- [x] Evil Portal with 8 templates
- [x] Deauth Attack
- [x] Beacon Flood
- [x] Probe Request Sniffer
- [x] Evil Twin
- [x] Auto-Portal
- [x] Automatic channel hopping
- [x] Web dashboard with real-time logs
- [ ] [CSV / JSON credential export](https://github.com/chrisq-dev/phantom-kit/issues/1)
- [ ] [Webhook notifications (Discord, Slack)](https://github.com/chrisq-dev/phantom-kit/issues/2)
- [ ] [Offline storage with SD card module](https://github.com/chrisq-dev/phantom-kit/issues/3)
- [ ] [WPA2 handshake capture (PMKID)](https://github.com/chrisq-dev/phantom-kit/issues/4)

Have a feature request? [Open an issue](https://github.com/chrisq-dev/phantom-kit/issues/new)

---

## Legal Disclaimer

This tool is **exclusively for educational purposes and authorized security audits**.

- Allowed: testing on your own networks, lab environments, audits with written authorization
- Not allowed: use on networks or devices without explicit owner authorization

Misuse of this tool may violate local and international laws. The author is not responsible for unauthorized use. See [docs/legal.md](docs/legal.md) for the full disclaimer.

---

## License

MIT License — see [LICENSE](LICENSE) for details.

---

<div align="center">

Developed for educational purposes and security awareness.

If this project was useful to you, consider starring it on GitHub.

</div>
