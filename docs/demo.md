# Guided Portfolio Demo

> [Versión en español](demo.es.md)

This demo script is designed for a portfolio walkthrough even when you do not have an ESP8266 on hand. It explains what the project does, how the operator flow works, and which safety controls make it suitable for authorized training.

## 60-Second Pitch

PhantomKit is a self-contained ESP8266 Wi-Fi security awareness platform. It runs its own management AP, captive DNS, web dashboard, portal templates, local storage, and Wi-Fi modules without a laptop-side server. The project demonstrates embedded security engineering, Wi-Fi protocol handling, constrained-memory design, and responsible tooling controls such as dashboard authentication, default credential blocking, passive mode, redacted demo output, emergency wipe, and release checksums.

## What To Show Without Hardware

Use the repository itself as the demo artifact:

1. Open `README.md` and explain the architecture diagram, threat model, and release assets.
2. Open `docs/safety.md` and highlight the passive audit profile, redaction, auth hardening, and emergency wipe.
3. Open `src/wifi/web_server.cpp` and point to:
   - `/api/audit` for the audit wizard.
   - `/api/passive` for passive-mode enforcement.
   - `rejectIfPassive()` for blocking active modules.
   - `handleAPIExportReport()` for redaction-aware reporting.
4. Open `.github/workflows/build.yml` and explain that tagged releases build firmware, LittleFS, and checksums.
5. Show the latest GitHub release and its `SHA256SUMS.txt`.

This is enough to demonstrate design and engineering judgment without pretending to run hardware live.

## Dashboard Walkthrough Script

Use these talking points with screenshots later, or as a narrated code walkthrough now:

1. **Login and setup:** The dashboard has a separate password from the AP password and refuses login while default `change-me-*` credentials are compiled in.
2. **Audit wizard:** The operator selects `passive`, `portal`, or `full` and records SSID/BSSID/channel scope.
3. **Passive mode:** In passive mode, PhantomKit blocks deauth, beacon flood, evil twin, auto-attack, portal start, and karma. Scans, probe sniffing, PMKID capture, logs, reports, and wipe remain available.
4. **Portal workflow:** For authorized awareness training, the operator selects a template, starts the captive portal, captures only test values, then exports a session report.
5. **Report:** The report includes scope, passive-mode state, counters, findings, captured-data status, and operational logs. With redaction enabled, raw captured fields are omitted.
6. **Cleanup:** The operator uses dashboard wipe or GPIO wipe before storing or handing off the device.

## Portfolio Explanation

Use this paragraph in a README section, personal site, or interview:

> PhantomKit is not presented as a generic attack gadget. It is framed as an authorized lab and awareness platform. The technical value is in the ESP8266 implementation: a constrained single-radio device running a local dashboard, captive DNS, LittleFS templates, Wi-Fi frame modules, PMKID parsing, reporting, and safety controls. The professional value is in the threat model, passive-mode guardrails, redacted demos, release checksums, and documentation that explain where the tool is appropriate and where it is not.

## Issue Roadmap Notes

The open issues are intentionally scoped as future work:

- `#4 PMKID`: current firmware captures/export PMKID hashcat lines; persistence to LittleFS/SD remains future work.
- `#3 SD card`: still open because SD auto-detect and log/credential mirroring are not implemented.
- `#2 Webhooks`: current firmware supports ntfy/custom HTTP webhook configuration; retry, toggle, and provider-specific Discord/Slack payload polish remain future work.

Keeping those issues open is better than closing them prematurely because it shows honest acceptance criteria.
