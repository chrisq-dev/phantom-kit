<div align="center">

[English](README.md) | [Español](README.es.md)

# ESP8266 PhantomKit

**Suite de auditoría Wi-Fi y pruebas de ingeniería social para ESP8266**

[![Licencia: MIT](https://img.shields.io/badge/Licencia-MIT-blue.svg)](LICENSE)
[![Plataforma](https://img.shields.io/badge/Plataforma-ESP8266-orange.svg)](https://www.espressif.com/en/products/socs/esp8266)
[![Framework](https://img.shields.io/badge/Framework-Arduino%20%2F%20PlatformIO-teal.svg)](https://platformio.org/)
[![Release](https://img.shields.io/github/v/release/chrisq-dev/phantom-kit?color=brightgreen)](https://github.com/chrisq-dev/phantom-kit/releases)
[![Issues](https://img.shields.io/github/issues/chrisq-dev/phantom-kit)](https://github.com/chrisq-dev/phantom-kit/issues)

*Convierte tu ESP8266 en una plataforma autónoma de auditoría de seguridad Wi-Fi.*  
*Sin infraestructura externa. Sin dependencias. Solo el chip.*

</div>

---

## Descripción

ESP8266 PhantomKit es una herramienta open-source de auditoría de seguridad Wi-Fi que corre completamente en un microcontrolador ESP8266 (NodeMCU, Wemos D1 Mini, etc.). Implementa técnicas reales de ingeniería social a través de un dashboard web accesible desde cualquier dispositivo con navegador, sin necesidad de laptop, servidor ni conexión a internet.

Diseñado para:
- Estudiantes de ciberseguridad que quieren aprender con hardware real
- Auditores que realizan pruebas de phishing en entornos controlados
- Capacitadores de concientización en seguridad corporativa

---

## Características

### Dashboard Web
- Control total desde cualquier dispositivo con navegador
- Captura de credenciales en tiempo real
- Logs de eventos en vivo con timestamp
- SSID configurable sin reprogramar
- Operación autónoma sin internet ni servidor externo

### Módulos de Ataque

| Módulo | Descripción | Técnica |
|--------|-------------|---------|
| Evil Portal | Portal cautivo con 8 templates clonados | DNS Spoofing + Captive Portal |
| Deauth Attack | Desconecta dispositivos de una red objetivo | IEEE 802.11 Deauth Frames |
| Beacon Flood | Satura listas de redes con 50+ SSIDs falsos | Fake Beacon Frames |
| Probe Sniffer | Detecta redes buscadas por dispositivos cercanos | Passive Probe Capture |
| Evil Twin | Clona una red Wi-Fi existente | SSID/BSSID Spoofing |
| Channel Hopping | Rotación automática en canales 1-13 | Multi-channel Scanning |
| Auto-Portal | Sugiere el mejor template según el entorno | Environment Analysis |

### Templates de Evil Portal

| Template | Estilo | Campos |
|----------|--------|--------|
| Facebook | Azul clásico, card centrada | Email + Contraseña |
| Instagram | Gradiente purple/orange | Usuario + Contraseña |
| Microsoft | Fluent Design | Email + Contraseña |
| X (Twitter) | Dark mode negro | Usuario + Contraseña |
| Google | Ultra minimalista | Email + Contraseña |
| WiFi Login | Genérico moderno | Contraseña Wi-Fi x2 |
| Netflix | Fondo oscuro, rojo | Email + Contraseña |
| WhatsApp | Verde, mobile-first | Teléfono + Código |

---

## Inicio Rápido

### Requisitos

- ESP8266 (NodeMCU v2, Wemos D1 Mini o similar)
- [PlatformIO](https://platformio.org/) instalado
- Cable USB de datos
- Python 3.x

### Instalación

```bash
# 1. Clonar el repositorio
git clone https://github.com/chrisq-dev/phantom-kit.git
cd phantom-kit

# 2. Compilar el firmware
pio run

# 3. Subir templates al sistema de archivos (LittleFS)
pio run --target uploadfs

# 4. Flashear el firmware
pio run --target upload

# 5. Conectarse a la red creada por el ESP8266
#    SSID:     PhantomKit
#    Password: phantom123

# 6. Abrir el dashboard en el navegador
#    http://192.168.4.1/dashboard
```

### Configuración

Edita `src/config.h` para cambiar el SSID, contraseña o canal antes de compilar:

```cpp
#define AP_SSID     "PhantomKit"
#define AP_PASSWORD "phantom123"
#define AP_CHANNEL  6
```

---

## Estructura del Proyecto

```
phantom-kit/
├── platformio.ini              # Configuración de PlatformIO
├── src/
│   ├── main.cpp                # Entry point
│   ├── config.h / config.cpp   # Configuración global
│   ├── wifi/
│   │   ├── ap_manager.*        # Gestión del Access Point
│   │   ├── dns_server.*        # DNS spoofing (captive portal)
│   │   ├── web_server.*        # Dashboard web + API REST
│   │   ├── deauth.*            # Módulo Deauth Attack
│   │   ├── beacon_flood.*      # Módulo Beacon Flood
│   │   ├── probe_sniffer.*     # Módulo Probe Sniffer
│   │   ├── evil_twin.*         # Módulo Evil Twin
│   │   ├── auto_portal.*       # Módulo Auto-Portal
│   │   └── channel_hopper.*    # Channel Hopping automático
│   └── portal/
│       ├── captive_portal.*    # Lógica del portal cautivo
│       └── credential_store.*  # Almacenamiento de credenciales
├── data/templates/             # Templates HTML (LittleFS)
│   ├── facebook.html
│   ├── instagram.html
│   ├── microsoft.html
│   ├── x.html
│   ├── google.html
│   ├── wifi_login.html
│   ├── netflix.html
│   └── whatsapp.html
├── docs/
│   ├── setup.md                # Guía de instalación detallada
│   ├── usage.md                # Guía de uso y escenarios
│   └── legal.md                # Disclaimer legal
├── CHANGELOG.md                # Historial de versiones
└── README.md
```

---

## Escenarios de Auditoría

### Escenario 1 — Oficina Corporativa
```
Objetivo:  Demostrar riesgo de phishing interno
SSID:      CorpWiFi-Visitantes
Template:  Microsoft Login
Módulos:   Evil Portal + Deauth sobre red corporativa
```

### Escenario 2 — Hotel / Aeropuerto
```
Objetivo:  Demostrar riesgo de redes públicas abiertas
SSID:      WiFi Gratis Hotel
Template:  WiFi Login
Módulos:   Evil Twin de la red del establecimiento
```

### Escenario 3 — Capacitación de Empleados
```
Objetivo:  Training de concientización en seguridad
SSID:      Free Airport WiFi
Template:  Google
Módulos:   Evil Portal con reporte de resultados
```

---

## Configuración Avanzada

### Channel Hopping
El ESP8266 rota automáticamente entre canales 1-13 para:
- Escanear todas las redes del área
- Capturar probe requests en cualquier canal
- Ejecutar ataques sobre objetivos en cualquier canal

### Canal del AP
Por defecto canal 6 (más compatible). Configurable en `config.h`:
```cpp
#define AP_CHANNEL 6
```

### Dependencias (PlatformIO)
```ini
lib_deps =
    ESP8266WiFi
    DNSServer
    ESP8266WebServer
    bblanchon/ArduinoJson @ ^6.21.3
```

---

## Roadmap

- [x] Evil Portal con 8 templates
- [x] Deauth Attack
- [x] Beacon Flood
- [x] Probe Request Sniffer
- [x] Evil Twin
- [x] Auto-Portal
- [x] Channel Hopping automático
- [x] Dashboard web con logs en tiempo real
- [x] Persistencia offline de credenciales (LittleFS)
- [x] [Exportar credenciales a CSV / JSON](https://github.com/chrisq-dev/phantom-kit/issues/1)
- [ ] [Notificaciones por webhook (Discord, Slack)](https://github.com/chrisq-dev/phantom-kit/issues/2)
- [ ] [Almacenamiento offline con módulo SD](https://github.com/chrisq-dev/phantom-kit/issues/3)
- [ ] [Captura de handshakes WPA2 (PMKID)](https://github.com/chrisq-dev/phantom-kit/issues/4)

¿Tienes una idea o feature request? [Abre un issue](https://github.com/chrisq-dev/phantom-kit/issues/new)

---

## Disclaimer Legal

Esta herramienta es exclusivamente para **fines educativos y auditorías de seguridad autorizadas**.

- Permitido: pruebas en redes propias, entornos de laboratorio, auditorías con autorización escrita
- No permitido: uso en redes o dispositivos sin autorización explícita del propietario

El uso indebido puede violar leyes locales e internacionales. El autor no se hace responsable por usos no autorizados. Consulta [docs/legal.md](docs/legal.md) para el disclaimer completo.

---

## Licencia

MIT License — consulta [LICENSE](LICENSE) para más detalles.

---

<div align="center">

Desarrollado con fines educativos y de concientización en ciberseguridad.

Si este proyecto te fue útil, considera darle una estrella en GitHub.

</div>
