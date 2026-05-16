# ESP8266 PhantomKit

> Suite de ingeniería social y auditoría de seguridad para ESP8266

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![ESP8266](https://img.shields.io/badge/platform-ESP8266-orange.svg)
![Status](https://img.shields.io/badge/status-active-success.svg)

## Descripción

ESP8266 PhantomKit es una herramienta de auditoría de seguridad que convierte tu ESP8266 en una plataforma autónoma de pruebas de ingeniería social. Con un dashboard web intuitivo y channel hopping automático, puedes operar en cualquier lugar sin depender de infraestructura externa.

## Características

- **Evil Portal** - Captive portal con 8 templates clonados (Facebook, Instagram, Microsoft, X, Google, WiFi Login, Netflix, WhatsApp)
- **Dashboard Web** - Interfaz responsive para control total desde cualquier dispositivo
- **Captura en Tiempo Real** - Credenciales visibles instantáneamente en el dashboard
- **SSID Configurable** - Cambia el nombre de la red sin reprogramar
- **Templates Intercambiables** - Selecciona el template apropiado para cada contexto
- **Logs en Vivo** - Monitoreo de eventos del sistema en tiempo real
- **Channel Hopping Automático** - Escaneo y ataque en canales 1-13 automáticamente
- **Operación Autónoma** - Funciona en cualquier lugar sin infraestructura externa
- **Deauth Attack** - Forzar desconexión de dispositivos de redes objetivo
- **Beacon Flood** - Generación de redes falsas para saturar listas WiFi
- **Probe Sniffer** - Detección de redes buscadas por dispositivos cercanos
- **Evil Twin** - Clonación de redes WiFi existentes

## Requisitos

- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
- PlatformIO Core
- Cable USB de datos
- Python 3.x

## Instalación Rápida

```bash
# 1. Clonar el repositorio
git clone https://github.com/tu-usuario/esp8266-phantomkit.git
cd esp8266-phantomkit

# 2. Compilar
pio run

# 3. Subir templates a LittleFS
pio run --target uploadfs

# 4. Flashear firmware
pio run --target upload

# 5. Conectarse a la red WiFi
# SSID: PhantomKit
# Password: phantom123

# 6. Abrir dashboard
# http://192.168.4.1/dashboard
```

## Estructura del Proyecto

```
esp8266-phantomkit/
├── platformio.ini              # Configuración de PlatformIO
├── src/
│   ├── main.cpp                # Entry point
│   ├── config.h                # Configuración global
│   ├── wifi/
│   │   ├── ap_manager.cpp/h    # Gestión de AP WiFi
│   │   ├── dns_server.cpp/h    # DNS spoofing
│   │   ├── web_server.cpp/h    # Dashboard web
│   │   ├── deauth.cpp/h        # Módulo Deauth
│   │   ├── beacon_flood.cpp/h  # Módulo Beacon Flood
│   │   ├── probe_sniffer.cpp/h # Módulo Probe Sniffer
│   │   ├── evil_twin.cpp/h     # Módulo Evil Twin
│   │   ├── auto_portal.cpp/h   # Módulo Auto-Portal
│   │   └── channel_hopper.cpp/h # Channel Hopping
│   └── portal/
│       ├── captive_portal.cpp/h # Portal cautivo
│       └── credential_store.cpp/h # Almacenamiento de credenciales
├── data/templates/             # Templates HTML
│   ├── facebook.html
│   ├── instagram.html
│   ├── microsoft.html
│   ├── x.html
│   ├── google.html
│   ├── wifi_login.html
│   ├── netflix.html
│   └── whatsapp.html
├── docs/
│   ├── setup.md                # Guía de instalación
│   ├── usage.md                # Guía de uso
│   └── legal.md                # Disclaimer legal
└── README.md
```

## Dashboard Web

El dashboard incluye:

- **Panel de Control** - Activar/desactivar portal, cambiar SSID, seleccionar template
- **Estadísticas** - Clientes conectados, credenciales capturadas, template activo
- **Tabla de Credenciales** - Capturas en tiempo real con template, campos y hora
- **Logs en Vivo** - Eventos del sistema con timestamp
- **Control de Ataques** - Iniciar/detener Deauth, Beacon, Probe, Evil Twin

## Templates

| Template | Estilo | Campos |
|----------|--------|--------|
| Facebook | Azul, card centrada | Email + Contraseña |
| Instagram | Minimalista, gradiente | Usuario + Contraseña |
| Microsoft | Fluent Design, limpio | Email + Contraseña |
| X (Twitter) | Dark mode, negro | Usuario + Contraseña |
| Google | Ultra minimalista | Email + Contraseña |
| WiFi Login | Genérico, moderno | Contraseña WiFi x2 |
| Netflix | Fondo oscuro, rojo | Email + Contraseña |
| WhatsApp | Verde, mobile-first | Teléfono + Código |

## Módulos de Ataque

### Deauth Attack
Envía paquetes de desautenticación para desconectar dispositivos de una red objetivo.
- Escaneo multi-canal (1-13)
- Selección por BSSID o canal
- Contador de frames enviados

### Beacon Flood
Genera redes WiFi falsas para saturar la lista de redes disponibles.
- 50+ SSIDs predefinidos
- Canal configurable
- Contador de beacons enviados

### Probe Sniffer
Captura probe requests para ver qué redes buscan los dispositivos cercanos.
- Channel hopping automático
- Detección de MAC y SSIDs buscados
- Historial de dispositivos

### Evil Twin
Clona una red WiFi existente para engañar a los usuarios.
- Escaneo multi-canal
- Clonación por SSID/BSSID
- Integración con Portal

### Auto-Portal
Detección automática de redes populares y configuración sugerida.
- Análisis de entorno
- Recomendaciones de template

## Uso en Auditorías

### Escenario 1: Oficina Corporativa
```
SSID: CorpWiFi-Visitantes
Template: Microsoft Login
Objetivo: Demostrar riesgo de phishing interno
```

### Escenario 2: Hotel/Aeropuerto
```
SSID: WiFi Gratis Hotel
Template: WiFi Login
Objetivo: Demostrar riesgo de redes públicas
```

### Escenario 3: Concientización
```
SSID: Free Airport WiFi
Template: Google
Objetivo: Training de seguridad para empleados
```

## Configuración Avanzada

### Channel Hopping
El ESP8266 rota automáticamente entre canales 1-13 para:
- Escanear todas las redes del área
- Capturar probes en cualquier canal
- Atacar objetivos en cualquier canal

### Canal del AP
Por defecto en canal 6 (más compatible). Configurable en `config.h`:
```cpp
#define AP_CHANNEL 6
```

## Roadmap

- [x] Deauth Attack
- [x] Beacon Flood
- [x] Probe Request Sniffer
- [x] Evil Twin
- [x] Auto-Portal
- [x] Channel Hopping
- [ ] Exportar credenciales (CSV/JSON)
- [ ] Notificaciones por webhook
- [ ] Modo offline con SD card
- [ ] Captura de handshakes WPA2

## Disclaimer Legal

Esta herramienta es exclusivamente para **fines educativos y auditorías de seguridad autorizadas**. El uso indebido es responsabilidad del usuario. Ver [docs/legal.md](docs/legal.md) para más detalles.

## Licencia

MIT License - Ver [LICENSE](LICENSE) para más detalles.

## Autor

Desarrollado con fines educativos y de concientización en ciberseguridad.
