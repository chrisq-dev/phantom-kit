# Architecture - ESP8266 PhantomKit

> [Versión en español](architecture.es.md)

PhantomKit is a self-contained Wi-Fi security awareness platform built for controlled labs and authorized audits. The ESP8266 runs the management access point, captive DNS, web dashboard, REST-style API, portal templates, storage layer, and Wi-Fi modules on a single device.

## Design Goals

- Run without external infrastructure after flashing
- Keep the operator workflow browser-based and device-agnostic
- Separate Wi-Fi modules so each feature can be started, stopped, and tested independently
- Preserve a portfolio-safe demo mode with redacted dashboard/API output
- Document legal and technical limitations clearly for responsible use

## System Overview

```text
Browser / Operator Device
        |
        | HTTP dashboard + API
        v
ESP8266 SoftAP: PhantomKit
        |
        +-- ESP8266WebServer
        |      +-- Dashboard UI
        |      +-- Auth/session cookie
        |      +-- Module control endpoints
        |      +-- Export endpoints
        |
        +-- DNSServer
        |      +-- Captive portal DNS responses
        |
        +-- LittleFS
        |      +-- HTML portal templates
        |      +-- Credential/session data
        |      +-- Notification config
        |
        +-- Wi-Fi modules
               +-- Deauth
               +-- Beacon flood
               +-- Probe sniffer
               +-- Evil twin
               +-- Channel hopper
               +-- Auto-portal
               +-- PMKID capture
```

## Boot Flow

1. `main.cpp` initializes serial logging and mounts LittleFS.
2. Stored credentials and notification settings are loaded.
3. Optional uplink Wi-Fi is started for NTP and webhooks.
4. The management AP starts with `AP_SSID`, `AP_PASSWORD`, and `AP_CHANNEL`.
5. DNS, channel hopping, Wi-Fi modules, and the dashboard web server start.
6. The captive portal attaches to the web server and serves templates from LittleFS.

## Dashboard And API Layer

`src/wifi/web_server.cpp` owns the browser UI and local control API. It handles:

- Dashboard authentication with a generated session token and HttpOnly cookie
- Module start/stop actions
- Status polling for dashboard counters
- Credential listing with redacted fields when `DASHBOARD_REDACT_CREDENTIALS` is enabled
- CSV export blocking while demo-safe redaction is active
- Redaction-aware session report export with scope, counters, findings, and logs
- PMKID export in hashcat 22000 format
- Emergency wipe endpoint for lab cleanup

Dynamic dashboard tables escape values before injecting them into HTML to reduce XSS risk from SSIDs, probe names, captured fields, and generated hash lines.

## Captive Portal Flow

1. The ESP8266 runs a SoftAP and DNS server.
2. DNS requests are redirected toward the local dashboard/portal address.
3. When the portal is active, unknown HTTP routes serve the selected template from `data/templates/`.
4. Submitted fields are stored by `CredentialStore`.
5. The dashboard polls `/api/credentials` and displays redacted values by default.

## Storage Model

LittleFS stores portal templates and runtime artifacts. The credential store keeps recent entries in memory and appends captured rows to `/credentials.csv`.

Important trade-off: LittleFS data is not encrypted. The project documents this as a physical-access limitation and includes emergency wipe controls for lab cleanup.

## Wi-Fi Module Boundaries

| Module | Responsibility |
|---|---|
| `APManager` | SoftAP lifecycle, SSID changes, stealth mode |
| `PhantomDNSServer` | Captive portal DNS behavior |
| `DeauthModule` | Target scan, client sniffing, deauth frame transmission |
| `BeaconFloodModule` | Fake SSID beacon generation |
| `ProbeSnifferModule` | Probe request capture, OUI lookup, Karma callback support |
| `EvilTwinModule` | Target scanning and AP cloning workflow |
| `AutoPortalModule` | Template suggestion based on target/environment keywords |
| `ChannelHopper` | Controlled channel rotation across 1-13 |
| `PMKIDCaptureModule` | EAPOL/RSN parsing and hashcat export generation |
| `CredentialStore` | In-memory and LittleFS-backed credential/session records |
| `NotifierModule` | Optional webhook/ntfy notifications |

## Resource Constraints

The ESP8266 has limited RAM and a single Wi-Fi radio. PhantomKit keeps the implementation compact, but these constraints shape the design:

- Some modules contend for channel control and promiscuous mode.
- Dashboard HTML/JS is embedded as program memory strings to avoid external assets.
- Large lookup tables use PROGMEM where practical.
- The current firmware builds near the upper RAM comfort zone, so new features should be measured with `pio run`.

## Security And Demo-Safety Controls

- Dashboard authentication is separate from the AP password.
- Default credentials are intentionally marked as `change-me-*`.
- `DASHBOARD_REDACT_CREDENTIALS` redacts captured fields in API/dashboard output.
- CSV export is disabled while redaction is enabled; session reports remain available with raw fields omitted.
- Emergency wipe removes sensitive LittleFS artifacts and restarts the device.
- Legal and responsible-use language is present in the README, SECURITY policy, and legal docs.

## Known Trade-Offs

- Session tokens are pseudo-random, suitable for local lab use but not cryptographic-grade authentication.
- LittleFS storage is plaintext.
- The ESP8266 radio cannot reliably support every module simultaneously.
- This project is intended for authorized labs and awareness training, not unsanctioned testing.

## Portfolio Talking Points

- Built a complete embedded security toolchain around a constrained microcontroller.
- Implemented modular Wi-Fi behavior around 802.11 management frames, probe requests, and EAPOL parsing.
- Designed a local web dashboard and API without relying on cloud services.
- Added responsible-use controls so demonstrations can be shown without exposing raw sensitive data.
- Balanced usability, safety, and resource constraints under real ESP8266 limits.
