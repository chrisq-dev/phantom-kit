# Usage Guide - ESP8266 PhantomKit

PhantomKit must only be used in owned networks, isolated labs, training environments, CTF-style exercises, or audits with explicit written authorization.

## Web Dashboard

After connecting to the ESP8266 AP, open:

```text
http://192.168.4.1/dashboard
```

The dashboard provides:

- Current AP SSID
- Portal template selection
- Portal start/stop control
- Module status counters
- Redacted captured-field table when demo mode is enabled
- Live event log
- Export controls when raw export mode is allowed

## Captive Portal

**Goal:** demonstrate credential-entry risk through controlled captive portal simulations.

Typical authorized lab flow:

1. Select a portal template.
2. Optionally set a lab SSID.
3. Start the portal.
4. Connect a test device owned by the operator.
5. Submit fake/demo values.
6. Review redacted results in the dashboard.
7. Wipe collected artifacts after the exercise.

## Deauth Module

**Goal:** demonstrate the impact of 802.11 deauthentication frames in a controlled RF environment.

Use only against a test AP or an explicitly authorized target:

1. Scan networks.
2. Select the target BSSID and channel.
3. Start the module.
4. Observe lab device behavior.
5. Stop the module and restore normal operation.

## Beacon Flood

**Goal:** demonstrate how fake beacon frames can clutter nearby Wi-Fi scan lists.

1. Select a channel.
2. Start the module.
3. Observe the scan list from a lab device.
4. Stop the module after the demonstration.

## Probe Sniffer

**Goal:** show how devices may reveal previously known SSIDs through probe requests.

1. Start sniffing.
2. Move lab devices near the ESP8266.
3. Review detected MACs, vendors, RSSI, and requested SSIDs.
4. Stop sniffing and clear artifacts after the lab.

## Evil Twin

**Goal:** demonstrate SSID cloning risk in a controlled lab.

1. Scan nearby networks.
2. Select a test SSID/BSSID.
3. Clone the AP name in the lab.
4. Optionally combine with the captive portal for awareness training.
5. Stop the clone when the exercise ends.

## PMKID Capture

**Goal:** demonstrate how WPA2 handshake material can be observed and exported for authorized password-audit workflows.

1. Start PMKID capture for a target BSSID or all BSSIDs.
2. Use a lab AP/client pair.
3. Export hashcat 22000 output only in an authorized audit context.
4. Stop capture and wipe artifacts after the exercise.

## Demo-Safe Workflow

For portfolio screenshots and videos:

1. Keep `DASHBOARD_REDACT_CREDENTIALS` enabled.
2. Use fake SSIDs and test credentials.
3. Show module controls and redacted results.
4. Avoid showing real nearby network names when possible.
5. Run emergency wipe before ending the demo.

## Reporting Workflow

For professional awareness exercises, document:

- Written authorization and scope
- Hardware and firmware version
- Date, time, and location of the lab
- Modules used
- Redacted evidence
- Risk explanation
- Remediation guidance
- Cleanup actions performed

## Troubleshooting

### No networks appear during scan

- Wait for multi-channel scanning to complete.
- Confirm nearby lab networks are active.
- Check logs for scan errors.

### Dashboard disconnects

- Some modules change channels temporarily.
- Reconnect to the management AP.
- Stop active modules if the dashboard becomes unstable.

### Export returns 403

CSV/report export is intentionally blocked while `DASHBOARD_REDACT_CREDENTIALS` is enabled. Disable redaction only inside an authorized lab when raw values are required.
