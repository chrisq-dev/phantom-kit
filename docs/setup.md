# Setup Guide - ESP8266 PhantomKit

## Requisitos

- PlatformIO Core instalado
- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
- Cable USB de datos
- Python 3.x

## Instalación

### 1. Instalar PlatformIO

```bash
pip install platformio
```

### 2. Verificar conexión del ESP8266

```bash
lsusb | grep -i ch340
ls -la /dev/ttyUSB*
```

Si no tienes permisos:
```bash
sudo chmod 666 /dev/ttyUSB0
```

### 3. Compilar el proyecto

```bash
cd esp8266-phantomkit
pio run
```

### 4. Subir templates a LittleFS

```bash
pio run --target uploadfs
```

### 5. Flashear el firmware

```bash
pio run --target upload
```

### 6. Abrir monitor serial

```bash
pio device monitor --baud 115200
```

## Uso

1. Conecta el ESP8266 por USB
2. Compila y flashea con los comandos anteriores
3. Desde tu teléfono o laptop, conéctate a la red WiFi:
   - **SSID:** `PhantomKit`
   - **Password:** `phantom123`
4. Abre el navegador y ve a: `http://192.168.4.1/dashboard`
5. Verás el dashboard de control con todos los módulos

## Configuración Avanzada

### Channel Hopping
El ESP8266 incluye channel hopping automático para escanear y operar en todos los canales (1-13). Esto permite:
- Detectar redes en cualquier canal
- Atacar objetivos en cualquier canal
- Operar de forma autónoma en cualquier ubicación

Configurable en `src/config.h`:
```cpp
#define CHANNEL_HOPPING_ENABLED true
#define CHANNEL_HOP_INTERVAL 500  // ms por canal
#define MIN_CHANNEL 1
#define MAX_CHANNEL 13
```

### Canal del AP
Por defecto en canal 6 (más compatible universalmente). Cambiable en `src/config.h`:
```cpp
#define AP_CHANNEL 6
```

### SSID y Password
Configurables en `src/config.h`:
```cpp
#define AP_SSID "PhantomKit"
#define AP_PASSWORD "phantom123"
```

## Módulos Incluidos

El firmware incluye todos los módulos activos:
- ✅ Portal Cautivo (8 templates)
- ✅ Deauth Attack
- ✅ Beacon Flood
- ✅ Probe Sniffer
- ✅ Evil Twin
- ✅ Auto-Portal
- ✅ Channel Hopping

## Troubleshooting

### El ESP8266 no aparece en `/dev/ttyUSB*`
- Verifica que el cable USB sea de datos (no solo carga)
- Prueba en otro puerto USB
- Instala drivers CH340 si es necesario

### Error al subir LittleFS
- Asegúrate de que los templates están en `data/templates/`
- Verifica que hay espacio suficiente en flash

### El dashboard no carga
- Verifica que estás conectado a la red `PhantomKit`
- Intenta `http://192.168.4.1/dashboard` directamente
- Hard refresh: `Ctrl+Shift+R`
- Revisa el monitor serial para errores

### Los módulos no funcionan
- Verifica que el firmware se subió correctamente
- Revisa los logs en el dashboard
- Reinicia el ESP8266 (desconecta y conecta USB)

### Pérdida de conexión durante ataques
- Algunos ataques cambian el canal temporalmente
- El ESP8266 vuelve al canal del AP automáticamente
- Reconecta al WiFi si es necesario

## Verificación de Instalación

Después de flashear, verifica que todo funcione:

1. **Monitor Serial:** Deberías ver:
   ```
   ESP8266 PhantomKit iniciando...
   LittleFS montado correctamente
   WiFi AP iniciado: PhantomKit Canal: 6
   DNS Server iniciado
   Channel Hopper iniciado
   [DEAUTH] Modulo iniciado
   [BEACON] Modulo iniciado con 51 SSIDs
   [PROBE] Modulo iniciado
   Dashboard web: http://192.168.4.1/dashboard
   PhantomKit listo. Conectate a: PhantomKit
   ```

2. **Dashboard:** Abre `http://192.168.4.1/dashboard` y verifica:
   - Header con logo "PK" y "PhantomKit"
   - 5 pestañas: Portal, Deauth, Beacon, Probe, Evil Twin
   - Logs mostrando mensajes de inicio
   - Indicador de estado "Inactivo"

3. **API:** Abre `http://192.168.4.1/api/status` y verifica:
   - JSON con estado de todos los módulos
   - `portal_active: false`
   - `template_name: "Facebook"`
   - Contadores en 0
