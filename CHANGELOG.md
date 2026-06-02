# Changelog

All notable changes to ESP8266 PhantomKit are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> [Versión en Español](CHANGELOG.es.md)

---

## [1.3.0] - 2026-06-02

### Added

#### Auto-Attack Chain
- New **Auto-Attack** tab in the dashboard — one click chains scan, SSID clone, unicast deauth, and portal activation with the best-fit template
- `POST /api/autoattack` endpoint with `action=start|stop|suggest`
- `suggestTemplate(ssid)` classifies any SSID by keyword to auto-select the most convincing portal template (ISP, corporate, social, streaming)
- `getTemplateNameFor(ssid)` returns the human-readable template name for a given SSID

#### Karma Attack
- `setKarmaMode(bool, KarmaCallback)` on `ProbeSnifferModule` — when enabled, triggers a callback on every new unique non-broadcast SSID probe
- `karmaCallback()` in `main.cpp` changes the softAP SSID to match the probed SSID and activates the portal automatically
- Toggle from the **Ajustes** tab via `POST /api/karma` (`action=start|stop`)
- Dashboard displays active karma SSID in real time via status poll

#### PMKID Capture
- New `PMKIDCaptureModule` — parses EAPOL Key frames in promiscuous mode to extract WPA2 PMKIDs from the first message of the 4-way handshake (no client association required; Jens Steube, 2018)
- Full RSN IE parser: locates PMKID Count field by traversing Pairwise Cipher Suite and AKM Suite counts
- Deduplication — same PMKID is not stored twice across captures
- `getHashcatOutput()` returns all captured PMKIDs in hashcat 22000 format (`WPA*02*...`)
- New **PMKID** tab in dashboard with start/stop controls and live results table
- `GET /api/pmkid/export` — downloads `.hc22000` file directly from the device

#### OUI Manufacturer Lookup
- New `oui.h` — PROGMEM lookup table covering Apple, Samsung, Google, Xiaomi, Huawei, Intel, OnePlus, Motorola, ASUS, LG, Sony, MediaTek, and Realtek OUI prefixes
- `lookupOUI(mac[3])` returns vendor name from the first 3 bytes of a MAC address
- `ProbeDevice.vendor` field populated on first detection in `addDevice()`
- Vendor column added to the probe sniffer device table in the dashboard

#### Emergency Wipe
- `GPIO_WIPE_PIN` (GPIO0 / NodeMCU FLASH button) — hold for `GPIO_WIPE_HOLD_MS` (default 3 s) to wipe all stored data and reboot
- `POST /api/panic` — unauthenticated web endpoint that removes `/credentials.csv` and `/notify.cfg` then reboots
- Emergency Wipe button in the **Ajustes** tab with a confirmation dialog

#### Real-time Credential Toast
- Dashboard JavaScript detects credential count increases on every 3-second status poll
- `showToast(msg)` — floating notification slides in from the top-right corner when a new credential is captured; auto-dismisses after 5 seconds
- Works on any active dashboard tab

#### Quick-Attack Button
- Each row in the probe sniffer device table now has an **Attack** button
- Clicking it pre-fills the Auto-Attack form with the device's first sought SSID and MAC address and switches to the Auto-Attack tab

### Changed
- `ProbeSnifferModule::getDevicesJSON()` now includes a `vendor` field in each device object
- `ProbeDevice` struct adds a `vendor: String` field
- Dashboard `setInterval` now updates karma SSID, PMKID count, and probe device count in a single status poll
- `PhantomWebServer` constructor now takes a `PMKIDCaptureModule&` parameter
- `suggestTemplate()` and `getTemplateNameFor()` are now `static` public methods on `AutoPortalModule`
- Version string updated to `1.3.0`

### Removed
- `scripts/release.sh` — obsolete one-off script for v1.0.0 release automation

---

## [1.2.0] - 2026-06-02

### Added

#### Dashboard Authentication
- Session-token-based authentication for all `/dashboard` and `/api/*` routes
- `generateSessionToken()` creates a random hex token on boot
- `POST /login` — validates `DASHBOARD_PASSWORD` (configurable in `config.h`) and sets `pk_session` `HttpOnly` cookie
- `GET /logout` — clears the session cookie
- All sensitive API endpoints redirect to `/login` if not authenticated
- Logout button in dashboard header

#### Real Probe Sniffer
- Complete rewrite of `probe_sniffer.cpp` using real 802.11 management frame parsing
- `IRAM_ATTR probePromiscCb()` extracts source MAC (frame bytes 46–51), SSID information element (offset 60+), and RSSI from the 36-byte `RxControl` header
- `addDevice()` deduplicates by MAC and per-device SSID list
- Dashboard shows device table: MAC, sought SSIDs, RSSI
- `GET /api/probe/devices` returns live device JSON

#### Unicast Deauth
- `scanClients()` in `DeauthModule` — passive 800 ms promiscuous sniff before each attack to discover associated client MACs
- `IRAM_ATTR deauthSniffCb()` identifies client MACs from 802.11 data frames (ToDS/FromDS bits)
- `update()` sends unicast AP→Client and Client→AP deauth frames for each tracked client in addition to the broadcast fallback

#### Webhook Notifications
- New `src/notifier.h/.cpp` — sends HTTP POST on every credential capture
- Supports ntfy.sh (plain-text body with topic path) and generic JSON webhooks
- Configuration saved to `/notify.cfg` on LittleFS, persists across reboots
- `POST /api/notify` with `action=save|test`
- **Ajustes** tab: URL and topic fields with Save and Test buttons

#### NTP Timestamps
- `configTime()` called on boot when `UPLINK_SSID` is set — syncs with `pool.ntp.org`
- `getTimestamp()` returns `YYYY-MM-DD HH:MM:SS` when synced, falls back to `T+HH:MM:SS` (time since boot)

#### Stealth Mode
- `ap_manager.setStealthMode(bool)` uses SDK `wifi_softap_set_config` to set `ssid_hidden = 1`
- `POST /api/stealth` — toggle without reflashing
- State displayed and controlled from **Ajustes** tab

#### GitHub Actions CI
- `.github/workflows/build.yml` — compiles firmware on every push and PR to `main`
- Build badge added to README files

### Changed
- WiFi mode switched to `WIFI_AP_STA` when `UPLINK_SSID` is configured
- `ICACHE_RAM_ATTR` replaced with `IRAM_ATTR` in all promiscuous callbacks (deprecated macro)
- All `snprintf` timestamp buffers increased to 32 bytes to resolve `-Wformat-truncation` warnings

### Added (config.h)
- `DASHBOARD_PASSWORD` — dashboard access password
- `UPLINK_SSID` / `UPLINK_PASSWORD` — optional STA-mode uplink for NTP and webhooks
- `NTP_SERVER`, `NTP_OFFSET`, `NTP_DST` — NTP configuration

---

## [1.1.0] - 2026-06-02

### Added
- **LittleFS Credential Storage** — persistent storage of captured credentials (`/credentials.csv`). Credentials survive device reboots and power cycles.
- **CSV and Report Export** — download credentials from dashboard as `.csv` or formatted session report (`phantomkit_report.txt`)
- **Enhanced Portal Templates:**
  - **Facebook** — password verification step, loading animation, custom favicon
  - **Microsoft** — two-step flow (email then password) with dynamic avatar
  - **WiFi Login** — redesigned as a corporate captive portal (Cisco/Aruba style) with a progress indicator

---

## [1.0.0] - 2026-06-01

### Initial Release

First stable release of ESP8266 PhantomKit — a self-contained Wi-Fi security auditing platform for the ESP8266 microcontroller.

### Added

#### Core Platform
- Autonomous operation — no external infrastructure required
- Web dashboard at `http://192.168.4.1/dashboard` with real-time credential table and live logs
- LittleFS filesystem for serving HTML templates from flash
- Captive portal with automatic DNS redirection
- Channel hopping across channels 1–13

#### Attack Modules
- **Evil Portal** — 8 cloned login templates (Facebook, Instagram, Microsoft, X, Google, WiFi Login, Netflix, WhatsApp)
- **Deauth Attack** — IEEE 802.11 deauthentication frames
- **Beacon Flood** — 50+ fake SSIDs
- **Probe Sniffer** — passive probe request capture
- **Evil Twin** — SSID/BSSID cloning
- **Auto-Portal** — environment scan and template recommendation

#### Dashboard
- Real-time credential table (template, fields, timestamp)
- Live event log
- Attack control panel (start/stop per module)
- SSID and template configuration without reflashing

---

[1.3.0]: https://github.com/chrisq-dev/phantom-kit/compare/v1.2.0...v1.3.0
[1.2.0]: https://github.com/chrisq-dev/phantom-kit/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/chrisq-dev/phantom-kit/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/chrisq-dev/phantom-kit/releases/tag/v1.0.0
