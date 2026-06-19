# Contributing to PhantomKit

Thank you for taking the time to contribute. PhantomKit is an open-source Wi-Fi auditing platform for ESP8266 and welcomes contributions from the security research and maker communities.

**Legal reminder:** All contributions must be intended for use in authorized security audits, research, or educational environments. See [SECURITY.md](SECURITY.md) for the responsible-use policy.

---

## Table of Contents

- [Getting Started](#getting-started)
- [Development Environment](#development-environment)
- [Project Structure](#project-structure)
- [Submitting Issues](#submitting-issues)
- [Submitting Pull Requests](#submitting-pull-requests)
- [Code Style](#code-style)
- [Commit Message Format](#commit-message-format)
- [Testing on Hardware](#testing-on-hardware)

---

## Getting Started

1. **Fork** the repository on GitHub
2. **Clone** your fork locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/phantom-kit.git
   cd phantom-kit
   ```
3. Create a **feature branch**:
   ```bash
   git checkout -b feat/my-new-feature
   ```
4. Make your changes, test on hardware, then open a PR.

---

## Development Environment

### Requirements

| Tool | Version | Install |
|------|---------|---------|
| PlatformIO Core | >= 6.x | `pip install platformio` |
| PlatformIO IDE | any | [VS Code extension](https://platformio.org/install/ide?install=vscode) |
| `gh` CLI | any | [cli.github.com](https://cli.github.com) |
| ESP8266 board | NodeMCU v2 / ESP-12E | — |

### First build

```bash
# Install dependencies and compile
pio run

# Flash filesystem (portal templates)
pio run --target uploadfs

# Flash firmware
pio run --target upload

# Monitor serial output
pio device monitor --baud 115200
```

### Configuration

Customize compile-time settings in `src/config.h`:

```cpp
#define AP_SSID            "PhantomKit"
#define AP_PASSWORD        "change-me-phantomkit"
#define DASHBOARD_PASSWORD "change-me-auditor"
#define UPLINK_SSID        ""          // Optional — enables NTP and webhooks
#define UPLINK_PASSWORD    ""
```

---

## Project Structure

```
phantom-kit/
├── src/
│   ├── main.cpp                    # Setup, loop, global instances
│   ├── config.h                    # All compile-time configuration
│   ├── notifier.h / .cpp           # Webhook notification module
│   ├── portal/
│   │   ├── captive_portal.h/.cpp   # Captive portal logic
│   │   └── credential_store.h/.cpp # LittleFS credential persistence
│   └── wifi/
│       ├── ap_manager.h/.cpp       # SoftAP management (stealth, SSID)
│       ├── auto_portal.h/.cpp      # SSID auto-match / template suggestion
│       ├── beacon_flood.h/.cpp     # Beacon flood attack
│       ├── channel_hopper.h/.cpp   # Channel hopping
│       ├── deauth.h/.cpp           # Deauth attack (broadcast + unicast)
│       ├── dns_server.h/.cpp       # Captive portal DNS
│       ├── evil_twin.h/.cpp        # Evil twin AP
│       ├── oui.h                   # OUI manufacturer lookup (PROGMEM)
│       ├── pmkid_capture.h/.cpp    # WPA2 PMKID capture
│       ├── probe_sniffer.h/.cpp    # 802.11 probe request sniffer + Karma
│       └── web_server.h/.cpp       # Dashboard HTTP server
├── data/
│   └── templates/                  # Portal HTML templates (LittleFS)
├── .github/
│   ├── workflows/build.yml         # CI — compile on every push
│   └── ISSUE_TEMPLATE/             # Issue templates
├── platformio.ini
├── CHANGELOG.md
├── CONTRIBUTING.md
├── SECURITY.md
└── CODE_OF_CONDUCT.md
```

---

## Submitting Issues

Before opening an issue:

1. Search existing issues to avoid duplicates.
2. Check the FAQ in the README.
3. Use the appropriate issue template (bug report or feature request).

A good bug report includes:
- Board model (NodeMCU v2, Wemos D1 Mini, etc.)
- PlatformIO version (`pio --version`)
- Full serial monitor output
- Steps to reproduce
- Expected vs actual behavior

---

## Submitting Pull Requests

### What we welcome

- Bug fixes
- New attack modules or portal templates
- Support for additional ESP8266/ESP32 boards
- Documentation improvements
- Translations
- Test utilities (lab scripts, packet capture analysis tools)

### PR checklist

- [ ] Tested on physical hardware (not just compiled)
- [ ] `pio run` completes with no errors
- [ ] RAM usage stays below 90% (shown in `pio run` output)
- [ ] New features are documented in a comment block or updated README section
- [ ] Commit messages follow the [Conventional Commits](#commit-message-format) format
- [ ] PR description explains *why* the change is needed, not just *what* it does

### Keeping PRs focused

One PR = one feature or fix. Large PRs that touch many unrelated areas will be asked to split.

---

## Code Style

PhantomKit is written in C++ (Arduino framework). Follow these conventions:

### Naming

```cpp
// Classes: PascalCase
class ProbeSnifferModule { ... };

// Methods and variables: camelCase
void startSniffing(int channel);
bool isRunning;

// Constants and macros: UPPER_SNAKE_CASE
#define AP_CHANNEL 6
#define MAX_CREDENTIALS 50

// Files: snake_case
probe_sniffer.cpp
```

### ESP8266 memory rules

The ESP8266 has 80 KB RAM. Keep this in mind:

- Store large constant tables in `PROGMEM` (see `oui.h` for an example)
- Use `FPSTR()` for long string literals served over HTTP
- Avoid dynamic allocations in hot paths (`new` / `malloc`)
- Promiscuous mode callbacks **must** be `IRAM_ATTR`
- Check RAM usage after every significant addition: target below 90%

### Module pattern

Every module follows this pattern:

```cpp
class MyModule {
public:
    MyModule();
    void begin();        // Initialize, log startup message
    void update();       // Called every loop() iteration
    bool isRunning() const;
private:
    bool running;
};
```

---

## Commit Message Format

PhantomKit uses [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <short description>

[optional body]
[optional footer]
```

### Types

| Type | When to use |
|------|-------------|
| `feat` | New feature or attack module |
| `fix` | Bug fix |
| `docs` | Documentation only |
| `refactor` | Code restructuring without behavior change |
| `perf` | Performance improvement |
| `chore` | Build process, dependencies, CI |
| `test` | Adding test utilities |

### Examples

```
feat(probe): add OUI manufacturer lookup from PROGMEM table
fix(deauth): stop promiscuous callback before channel switch
docs(readme): add wiring diagram for NodeMCU v2
chore(ci): pin PlatformIO version to 6.1.11
```

---

## Testing on Hardware

PhantomKit has no automated unit tests — firmware for embedded targets is difficult to mock. All testing is manual and must be done in a controlled lab environment.

### Recommended lab setup

```
[Laptop (monitor)]  --USB-->  [NodeMCU (PhantomKit)]  --WiFi-->  [Test phone (victim)]
```

- Use a dedicated test phone or a virtual machine with a WiFi adapter
- Never test on networks you do not own or have explicit written permission to audit
- For PMKID tests, use a home router you control
- For Evil Twin / Karma tests, use a hotspot from your own phone

### Verifying a build

```bash
pio run                          # Must succeed with < 90% RAM
pio run --target uploadfs        # Flash templates
pio run --target upload          # Flash firmware
pio device monitor --baud 115200 # Check boot log for errors
```

Expected boot output:
```
[T+00:00:01] ESP8266 PhantomKit iniciando...
[T+00:00:01] Version 1.3.0
[T+00:00:02] WiFi AP iniciado: PhantomKit Canal: 6
[T+00:00:02] Dashboard web: http://192.168.4.1/dashboard
[T+00:00:02] PhantomKit listo.
```

---

## Questions

Open a [Discussion](https://github.com/chrisq-dev/phantom-kit/discussions) or reach out via the issue tracker.
