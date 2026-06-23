# Setup Guide - ESP8266 PhantomKit

## Requirements

- PlatformIO Core installed
- ESP8266 board (NodeMCU, Wemos D1 Mini, or similar)
- USB data cable
- Python 3.x

## Installation

### 1. Install PlatformIO

```bash
pip install platformio
```

### 2. Verify ESP8266 connection

```bash
lsusb | grep -i ch340
ls -la /dev/ttyUSB*
```

If your user cannot access the serial device:

```bash
sudo chmod 666 /dev/ttyUSB0
```

### 3. Build the project

```bash
cd phantom-kit
pio run
```

### 4. Upload templates to LittleFS

```bash
pio run --target uploadfs
```

### 5. Flash the firmware

```bash
pio run --target upload
```

### 6. Open the serial monitor

```bash
pio device monitor --baud 115200
```

## First Use

1. Connect the ESP8266 over USB.
2. Build and flash the firmware using the commands above.
3. From a phone or laptop, connect to the Wi-Fi network:
   - **SSID:** `PhantomKit`
   - **Password:** the `AP_PASSWORD` value you configured in `src/config.h`
4. Open `http://192.168.4.1/dashboard` in a browser.
5. Log in with the dashboard password configured in `src/config.h`.

## Configuration

### Channel Hopping

The ESP8266 can rotate across channels 1-13 for scanning and module operation:

```cpp
#define CHANNEL_HOPPING_ENABLED true
#define CHANNEL_HOP_INTERVAL 500
#define MIN_CHANNEL 1
#define MAX_CHANNEL 13
```

### Management AP

Configured in `src/config.h`:

```cpp
#define AP_SSID "PhantomKit"
#define AP_PASSWORD "replace-with-a-strong-ap-password"
#define DASHBOARD_PASSWORD "replace-with-a-strong-dashboard-password"
```

The firmware blocks dashboard login while the default `change-me-*` values are compiled in.

Change these values before using the device outside a private lab.

### Demo-Safe Redaction

For portfolio demos, keep credential redaction enabled:

```cpp
#define DASHBOARD_REDACT_CREDENTIALS 1
```

When enabled, dashboard/API credential fields are redacted and raw CSV export is blocked. Session reports remain available with sensitive fields omitted.

## Included Modules

- Captive portal with 8 templates
- Deauth module
- Beacon flood module
- Probe sniffer
- Evil twin workflow
- Auto-portal template suggestion
- Channel hopping
- PMKID capture
- Emergency wipe

## Troubleshooting

### ESP8266 does not appear as `/dev/ttyUSB*`

- Verify that the USB cable supports data.
- Try another USB port.
- Install CH340 drivers if needed.

### LittleFS upload fails

- Confirm that templates exist under `data/templates/`.
- Check that the board has enough flash space.

### Dashboard does not load

- Confirm that you are connected to the `PhantomKit` Wi-Fi network.
- Open `http://192.168.4.1/dashboard` directly.
- Hard refresh the browser.
- Check the serial monitor for boot errors.

### Modules do not behave as expected

- Confirm that the firmware flashed successfully.
- Verify that the target channel matches the selected module.
- Check dashboard logs and serial output.
- Restart the ESP8266 after testing disruptive modules.

## Installation Verification

After flashing, the serial monitor should show the AP, DNS server, channel hopper, modules, and dashboard starting successfully.

The dashboard should show the main tabs, module counters, startup logs, and inactive/default module state.
