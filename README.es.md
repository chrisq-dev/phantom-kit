<div align="center">

[English](README.md) | [Español](README.es.md)

# ESP8266 PhantomKit

**Plataforma autorizada de concientización y auditoría Wi-Fi de laboratorio para ESP8266**

[![Licencia: MIT](https://img.shields.io/badge/Licencia-MIT-blue.svg)](LICENSE)
[![Plataforma](https://img.shields.io/badge/Plataforma-ESP8266-orange.svg)](https://www.espressif.com/en/products/socs/esp8266)
[![Framework](https://img.shields.io/badge/Framework-Arduino%20%2F%20PlatformIO-teal.svg)](https://platformio.org/)
[![Release](https://img.shields.io/github/v/release/chrisq-dev/phantom-kit?color=brightgreen)](https://github.com/chrisq-dev/phantom-kit/releases)
[![Issues](https://img.shields.io/github/issues/chrisq-dev/phantom-kit)](https://github.com/chrisq-dev/phantom-kit/issues)
[![Build](https://github.com/chrisq-dev/phantom-kit/actions/workflows/build.yml/badge.svg)](https://github.com/chrisq-dev/phantom-kit/actions/workflows/build.yml)

*Convierte tu ESP8266 en una plataforma autónoma de auditoría de seguridad Wi-Fi.*  
*Sin infraestructura externa. Sin dependencias. Solo el chip.*

</div>

---

## Descripción

ESP8266 PhantomKit es una herramienta open-source de concientización y auditoría Wi-Fi autorizada que corre completamente en un microcontrolador ESP8266 (NodeMCU, Wemos D1 Mini, etc.). Demuestra riesgos reales de Wi-Fi y portales cautivos a través de un dashboard web accesible desde cualquier dispositivo con navegador, sin necesidad de laptop, servidor ni conexión a internet.

Diseñado para:
- Estudiantes de ciberseguridad que quieren aprender con hardware real
- Auditores que realizan simulaciones de concientización en entornos autorizados
- Capacitadores de concientización en seguridad corporativa

PhantomKit está diseñado para laboratorios controlados, redes propias, ejercicios tipo CTF y auditorías con alcance escrito. No está pensado para usarse contra redes o usuarios de terceros.

---

## Valor para Portafolio

Este proyecto demuestra habilidades prácticas en:

- Seguridad embebida con ESP8266, PlatformIO, C++, LittleFS y límites reales de RAM/flash
- Protocolos Wi-Fi con frames 802.11, probe requests, channel hopping, manejo de BSSID/SSID y parsing de PMKID
- Diseño de tooling de seguridad con dashboard local, APIs autenticadas, logs, exportación y módulos separados
- Comunicación responsable en ciberseguridad con alcance legal, limitaciones documentadas, CI, guías de contribución y documentación bilingüe

Para demos, mantén `DASHBOARD_REDACT_CREDENTIALS` activado para que screenshots y respuestas API muestren datos redactados en vez de valores sensibles capturados. Las exportaciones CSV/reporte quedan desactivadas mientras este modo esta activo.

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
#    Password: change-me-phantomkit

# 6. Abrir el dashboard en el navegador
#    http://192.168.4.1/dashboard
```

### Configuración

Edita `src/config.h` para cambiar el SSID, contraseña o canal antes de compilar:

```cpp
#define AP_SSID     "PhantomKit"
#define AP_PASSWORD "change-me-phantomkit"
#define AP_CHANNEL  6
```

Cambia `AP_PASSWORD` y `DASHBOARD_PASSWORD` antes de flashear cualquier dispositivo usado fuera de un laboratorio privado.

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

### Completado

| Version | Feature |
|---------|---------|
| v1.0 | Evil Portal con 8 templates (Facebook, Instagram, Microsoft, X, Google, WiFi Login, Netflix, WhatsApp) |
| v1.0 | Deauth Attack — frames de deautenticacion IEEE 802.11 |
| v1.0 | Beacon Flood — mas de 50 SSIDs falsos |
| v1.0 | Probe Request Sniffer — captura pasiva |
| v1.0 | Evil Twin — clonacion de SSID/BSSID |
| v1.0 | Auto-Portal — escaneo del entorno y recomendacion de template |
| v1.0 | Channel Hopping automatico (canales 1 al 13) |
| v1.0 | Dashboard web con tabla de credenciales y logs en tiempo real |
| v1.1 | Persistencia offline de credenciales (LittleFS) |
| v1.1 | Exportacion a CSV y reporte de sesion |
| v1.2 | Autenticacion del dashboard (token de sesion, cookie HttpOnly) |
| v1.2 | Parsing real de frames 802.11 en el Probe Sniffer |
| v1.2 | Deauth Unicast (frames dirigidos AP→Cliente y Cliente→AP) |
| v1.2 | Notificaciones webhook (ntfy.sh y endpoints personalizados) |
| v1.2 | Timestamps reales via NTP |
| v1.2 | Modo Stealth (SSID de gestion oculto) |
| v1.2 | CI con GitHub Actions |
| v1.3 | Auto-Attack Chain (un clic: escaneo, deauth y portal) |
| v1.3 | Karma Attack (clonado automatico de SSIDs detectados en probes) |
| v1.3 | Captura de PMKID — exportacion en formato hashcat 22000 |
| v1.3 | Lookup de fabricante OUI (tabla en PROGMEM) |
| v1.3 | Portal Auto-Match (seleccion de template por palabras clave) |
| v1.3 | Emergency Wipe (mantener GPIO0 + endpoint /api/panic) |
| v1.3 | Notificacion toast de credenciales en tiempo real |

### Planificado

**v1.4 — Tooling e integracion**

- [ ] `phantomkit-pull` — CLI en Python para descargar PMKIDs y credenciales del dashboard y pasarlos directamente a hashcat o un archivo local
- [ ] Integracion con bot de Telegram — alertas de credenciales en tiempo real con mensajes formateados
- [ ] Guia de bateria + TP4056 — operacion portatil completamente autonoma
- [ ] Soporte de modulo SD — almacenamiento de credenciales independiente del tamano de LittleFS

**v2.0 — Puerto a ESP32**

- [ ] Portar el firmware a ESP32 — 520 KB de RAM, doble nucleo, elimina el techo de memoria actual
- [ ] Escaneo BLE — detectar dispositivos Bluetooth (auriculares, teclados, relojes inteligentes) junto con probes Wi-Fi
- [ ] Deauth y portal simultaneos — ejecutar ambos en nucleos separados sin conflictos de modo promiscuo
- [ ] Tabla OUI mas grande y mas templates de portal sin restricciones de PROGMEM

¿Tienes una idea o feature request? [Abre un issue](https://github.com/chrisq-dev/phantom-kit/issues/new/choose)

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
