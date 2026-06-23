<div align="center">

[English](README.md) | [Español](README.es.md)

# ESP8266 PhantomKit

**Authorized Wi-Fi security awareness and lab auditing platform for ESP8266**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ESP8266-orange.svg)](https://www.espressif.com/en/products/socs/esp8266)
[![Framework](https://img.shields.io/badge/Framework-Arduino%20%2F%20PlatformIO-teal.svg)](https://platformio.org/)
[![Release](https://img.shields.io/github/v/release/chrisq-dev/phantom-kit?color=brightgreen)](https://github.com/chrisq-dev/phantom-kit/releases)
[![Issues](https://img.shields.io/github/issues/chrisq-dev/phantom-kit)](https://github.com/chrisq-dev/phantom-kit/issues)
[![Build](https://github.com/chrisq-dev/phantom-kit/actions/workflows/build.yml/badge.svg)](https://github.com/chrisq-dev/phantom-kit/actions/workflows/build.yml)

*Turn your ESP8266 into a self-contained Wi-Fi security auditing platform.*  
*No external infrastructure. No dependencies. Just the chip.*

</div>

---

## What is PhantomKit?

ESP8266 PhantomKit is an open-source Wi-Fi security awareness and authorized lab auditing tool that runs entirely on an ESP8266 microcontroller (NodeMCU, Wemos D1 Mini, etc.). It demonstrates real Wi-Fi and captive portal risks through a web dashboard accessible from any browser-enabled device, with no laptop, server, or internet connection required.

Designed for:
- Security students who want to learn with real hardware
- Auditors conducting authorized awareness simulations
- Security awareness trainers in corporate environments

PhantomKit is built for controlled labs, owned networks, CTF-style exercises, and written-scope audits. It is not intended for use against third-party networks or users.

---

## Portfolio Value

This project demonstrates practical skills across:

- Embedded security engineering with ESP8266, PlatformIO, C++, LittleFS, and constrained RAM/flash budgets
- Wi-Fi protocol work with 802.11 management frames, probe requests, channel hopping, BSSID/SSID handling, and PMKID parsing
- Security tooling design through a local dashboard, authenticated APIs, logging, export flows, and modular attack components
- Responsible security communication with legal scope, documented limitations, CI, contribution guidelines, and bilingual documentation

For demos, keep `DASHBOARD_REDACT_CREDENTIALS` enabled so screenshots and API responses show redacted sample values instead of sensitive captured data. CSV/report exports are disabled while this mode is active.

### What This Project Shows

- I can design security tooling that balances offensive simulation with defensive, authorized-use controls
- I can work close to hardware constraints instead of relying only on desktop scripts or cloud services
- I can explain trade-offs clearly: memory limits, single-radio behavior, plaintext local storage, and local-only authentication
- I can maintain professional open-source hygiene with bilingual docs, CI, security policy, changelog, and contribution guidance

---

## Architecture At A Glance

```text
Operator Browser
   |
   | dashboard HTTP + local API
   v
ESP8266 SoftAP + Captive DNS
   |
   +-- Dashboard auth, session cookie, safety controls
   +-- LittleFS templates, redacted credential view, cleanup controls
   +-- Wi-Fi modules: portal, probe, PMKID, deauth, beacon, evil twin
   +-- Optional uplink for NTP and webhook notifications
```

The implementation keeps the experience fully local after flashing: no cloud service, no laptop-side server, and no internet requirement for normal lab operation. See [Architecture](docs/architecture.md) for the full module map and resource trade-offs.

## Threat Model

PhantomKit assumes a short-range lab environment where the operator controls the device and has written authorization for the RF space being tested.

| Asset | Risk | Mitigation |
|---|---|---|
| Dashboard access | Unauthorized local client controls modules | Separate dashboard password, default-credential guard, lockout, HttpOnly session cookie |
| Captured inputs | Sensitive values exposed in demos/logs | Redacted dashboard/API mode, blocked exports while redacted, serial redaction |
| Stored artifacts | LittleFS data readable with physical access | Emergency wipe, GPIO wipe, documented plaintext limitation |
| Nearby networks | Accidental disruption outside scope | Legal docs, usage scope, module start/stop boundaries, operator checklist |
| Release integrity | Unreviewed local firmware distributed | GitHub Actions release assets plus SHA-256 checksums |

Safety controls are documented in [Safety by Design](docs/safety.md).

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

# 2. Edit src/config.h and change AP_PASSWORD + DASHBOARD_PASSWORD

# 3. Build the firmware
pio run

# 4. Upload templates to the filesystem (LittleFS)
pio run --target uploadfs

# 5. Flash the firmware
pio run --target upload

# 6. Connect to the network created by the ESP8266
#    SSID:     PhantomKit
#    Password: the AP_PASSWORD value you configured

# 7. Open the dashboard in your browser
#    http://192.168.4.1/dashboard
```

### Configuration

Edit `src/config.h` to change the SSID, password, or channel before compiling:

```cpp
#define AP_SSID     "PhantomKit"
#define AP_PASSWORD "replace-with-a-strong-ap-password"
#define AP_CHANNEL  6
#define DASHBOARD_PASSWORD "replace-with-a-strong-dashboard-password"
```

The dashboard refuses login while the default `change-me-*` values are still compiled in. Change `AP_PASSWORD` and `DASHBOARD_PASSWORD` before flashing.

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
│   ├── architecture.md         # System architecture and design trade-offs
│   ├── architecture.es.md      # Arquitectura del sistema en español
│   ├── setup.md / setup.es.md  # Installation guides
│   ├── usage.md / usage.es.md  # Usage guides and scenarios
│   ├── safety.md / safety.es.md # Safety controls and operator checklist
│   └── legal.md / legal.es.md  # Legal disclaimers
├── CHANGELOG.md                # Version history
└── README.md
```

---

## Documentation

- [Architecture](docs/architecture.md)
- [Setup guide](docs/setup.md)
- [Usage guide](docs/usage.md)
- [Safety by Design](docs/safety.md)
- [Legal and ethical disclaimer](docs/legal.md)
- [Security policy](SECURITY.md)
- [Contributing guide](CONTRIBUTING.md)

Spanish documentation is available through [README.es.md](README.es.md), [docs/architecture.es.md](docs/architecture.es.md), [docs/setup.es.md](docs/setup.es.md), [docs/usage.es.md](docs/usage.es.md), [docs/safety.es.md](docs/safety.es.md), and [docs/legal.es.md](docs/legal.es.md).

## Releases

Tagged releases (`v*.*.*`) are built by GitHub Actions. Each release includes:

- `phantomkit-<version>-firmware.bin`
- `phantomkit-<version>-littlefs.bin`
- `SHA256SUMS.txt`

Use the checksum file to verify downloaded binaries before flashing.

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

### Completed

| Version | Feature |
|---------|--------|
| v1.0 | Evil Portal with 8 templates (Facebook, Instagram, Microsoft, X, Google, WiFi Login, Netflix, WhatsApp) |
| v1.0 | Deauth Attack — IEEE 802.11 deauthentication frames |
| v1.0 | Beacon Flood — 50+ fake SSIDs |
| v1.0 | Probe Request Sniffer — passive capture |
| v1.0 | Evil Twin — SSID/BSSID cloning |
| v1.0 | Auto-Portal — environment scan and template recommendation |
| v1.0 | Automatic channel hopping (channels 1–13) |
| v1.0 | Web dashboard with real-time credential table and logs |
| v1.1 | Offline credential persistence (LittleFS) |
| v1.1 | CSV and session report export |
| v1.2 | Dashboard authentication (session token, HttpOnly cookie) |
| v1.2 | Real 802.11 frame parsing in Probe Sniffer |
| v1.2 | Unicast Deauth (targeted AP→Client and Client→AP frames) |
| v1.2 | Webhook notifications (ntfy.sh and custom endpoints) |
| v1.2 | NTP real timestamps |
| v1.2 | Stealth Mode (hidden management SSID) |
| v1.2 | GitHub Actions CI |
| v1.3 | Auto-Attack Chain (one-click: scan, deauth, portal) |
| v1.3 | Karma Attack (auto-clone probed SSIDs) |
| v1.3 | PMKID Capture — hashcat 22000 export |
| v1.3 | OUI Manufacturer Lookup (PROGMEM table) |
| v1.3 | Portal Auto-Match (keyword-based template selection) |
| v1.3 | Emergency Wipe (GPIO0 hold + /api/panic endpoint) |
| v1.3 | Real-time credential toast notification |

### Planned

**v1.4 — Tooling and integration**

- [ ] `phantomkit-pull` — Python CLI to pull captured PMKIDs and credentials from the dashboard and pipe directly into hashcat or a local file
- [ ] Telegram bot integration — real-time credential alerts with formatted messages
- [ ] Battery + TP4056 wiring guide — fully portable standalone operation
- [ ] SD card module support — offline credential storage independent of LittleFS size

**v2.0 — ESP32 port**

- [ ] Port firmware to ESP32 — 520 KB RAM, dual core, eliminates the current memory ceiling
- [ ] BLE scanning — detect Bluetooth devices (headphones, keyboards, smartwatches) alongside Wi-Fi probes
- [ ] Simultaneous deauth + portal — run both on separate cores without promiscuous mode conflicts
- [ ] Larger OUI table and more portal templates without PROGMEM constraints

Have a feature request? [Open an issue](https://github.com/chrisq-dev/phantom-kit/issues/new/choose)

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
