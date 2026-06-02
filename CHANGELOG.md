# Changelog

All notable changes to ESP8266 PhantomKit are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.0.0] - 2026-06-01

### Initial Release

This is the first stable release of ESP8266 PhantomKit — a self-contained Wi-Fi security auditing platform built on the ESP8266 microcontroller.

### Added

#### Core Platform
- **Autonomous operation** — no external infrastructure required; runs fully on-device
- **Web dashboard** at `http://192.168.4.1/dashboard` with real-time credential table and live system logs
- **LittleFS** filesystem for serving HTML templates directly from flash memory
- **Captive portal** with automatic DNS redirection (all domains → portal page)
- **Channel hopping** — automatic rotation across channels 1–13 for scan/attack coverage

#### Attack Modules
- **Evil Portal** — 8 cloned login templates (Facebook, Instagram, Microsoft, X, Google, WiFi Login, Netflix, WhatsApp)
- **Deauth Attack** — sends IEEE 802.11 deauthentication frames to disconnect clients from a target AP
- **Beacon Flood** — broadcasts 50+ fake SSIDs to saturate nearby device scan lists
- **Probe Sniffer** — passively captures probe requests to discover networks sought by nearby devices
- **Evil Twin** — clones an existing SSID/BSSID to lure clients into a rogue AP
- **Auto-Portal** — scans the environment and recommends the most effective template

#### Dashboard Features
- Real-time credential capture table (template, fields, timestamp)
- Live event log with timestamps
- Attack control panel (start/stop per module)
- SSID configuration without reflashing
- Template switcher

#### Configuration
- `config.h` — centralized compile-time configuration (AP channel, SSID, password, etc.)
- PlatformIO build system with LittleFS filesystem support
- Serial monitor at 115200 baud for debug output

#### Documentation
- `docs/setup.md` — step-by-step installation guide
- `docs/usage.md` — usage guide with scenario walkthroughs
- `docs/legal.md` — legal disclaimer and responsible use guidelines

---

## [1.1.0] - 2026-06-02

### Added
- **LittleFS Credential Storage** — persistent storage of captured credentials on the ESP8266 internal flash memory (`/credentials.csv`). Captured credentials now persist across device reboots and power cycles.
- **CSV and Report Export** — download captured credentials directly from the Web UI dashboard as a `.csv` file or a formatted session report (`phantomkit_report.txt`).
- **Enhanced Portal Templates**:
  - **Facebook** — added a realistic password verification step, loading animation, and custom favicon.
  - **Microsoft** — implemented a realistic two-step credentials entry flow (email then password) with a dynamic avatar.
  - **WiFi Login** — redesigned as a corporate captive portal (Cisco/Aruba style) with a verification progress indicator.

---

## [Unreleased]

Features planned for upcoming releases:

- [ ] **Webhook notifications** — push captures to Discord, Slack, or custom endpoint
- [ ] **SD card support** — offline credential storage on SD module
- [ ] **WPA2 handshake capture** — passive PMKID / 4-way handshake sniffing
- [ ] **Multi-language portal templates** — localized versions of existing templates
- [ ] **OTA updates** — over-the-air firmware updates via dashboard

---

[1.1.0]: https://github.com/chrisq-dev/phantom-kit/releases/tag/v1.1.0
[1.0.0]: https://github.com/chrisq-dev/phantom-kit/releases/tag/v1.0.0
