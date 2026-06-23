# Guía de Uso - ESP8266 PhantomKit

> [English version](usage.md)

## Dashboard Web

Una vez conectado al AP del ESP8266, abre `http://192.168.4.1/dashboard` en tu navegador.

### Panel de Control

- **SSID Actual:** Muestra el nombre de la red WiFi activa
- **Cambiar SSID:** Escribe un nuevo nombre y aplica
- **Template:** Selecciona qué página de login mostrar
- **Activar/Desactivar Portal:** Controla el captive portal

### Estadísticas

- **Clientes:** Número de dispositivos conectados al AP
- **Credenciales:** Cantidad de credenciales capturadas
- **Template:** Template actualmente activo
- **Estado de módulos:** Indicadores visuales de actividad

### Credenciales Capturadas

Muestra en tiempo real todas las credenciales capturadas:
- Template usado
- Campo 1 (email/teléfono/usuario)
- Campo 2 (contraseña/código)
- Hora de captura

### Logs en Tiempo Real

Muestra todos los eventos del sistema con timestamp.

## Wizard de Auditoría

La pestaña **Auditoría** permite fijar el alcance antes de iniciar módulos:

1. Selecciona un perfil:
   - `passive`: bloquea deauth, beacon flood, evil twin, auto-attack, inicio de portal y karma.
   - `portal`: prepara un ejercicio de portal cautivo autorizado sin activar deauth.
   - `full`: habilita todos los módulos para laboratorio aislado.
2. Ingresa SSID/BSSID y canal autorizados cuando los conozcas.
3. Inicia la auditoría y revisa el log operativo.
4. Exporta el reporte de sesión antes de borrar artefactos.

El modo pasivo mantiene disponibles escaneos, probe sniffer, captura PMKID, reportes, logs y limpieza.

## Reportes

El reporte integrado incluye alcance actual, estado de modo pasivo, contadores, observaciones, estado de datos capturados y log operativo. Cuando `DASHBOARD_REDACT_CREDENTIALS` está activado, el reporte conserva conteos y metadatos de evidencia, pero no incluye campos capturados crudos.

La exportación CSV sigue bloqueada mientras la redacción está activa. Desactiva redacción solo dentro de un laboratorio autorizado cuando se requieren valores crudos.

## Módulos de Ataque

### 1. Portal (Captive Portal)
**Objetivo:** Capturar credenciales mediante páginas de login clonadas.

**Cómo usar:**
1. Selecciona un template (Facebook, Instagram, etc.)
2. Opcional: Cambia el SSID a algo creíble
3. Clic en **Activar**
4. Los usuarios que se conecten verán la página de login
5. Las credenciales aparecen en la tabla en tiempo real

**Flujo típico:**
- Usuario se conecta al WiFi "PhantomKit"
- Se abre automáticamente la página de login
- Usuario ingresa credenciales
- Se muestra mensaje "Conectando..."
- Credenciales capturadas en dashboard

### 2. Deauth Attack
**Objetivo:** Desconectar dispositivos de una red WiFi objetivo.

**Cómo usar:**
1. Clic en **Escanear Redes** (escanea canales 1-13 automáticamente)
2. Selecciona la red objetivo de la lista
3. Verifica el BSSID y canal
4. Clic en **Iniciar**
5. Los dispositivos se desconectarán de esa red
6. Clic en **Detener** para restaurar

**Notas:**
- El escaneo multi-canal encuentra redes en todos los canales
- El ataque funciona en el canal de la red objetivo
- Solo afecta la red seleccionada

### 3. Beacon Flood
**Objetivo:** Saturar la lista de WiFi con redes falsas.

**Cómo usar:**
1. Selecciona un canal (1-13)
2. Clic en **Iniciar**
3. Los dispositivos cercanos verán decenas de redes nuevas
4. Clic en **Detener** para limpiar

**Características:**
- SSIDs predefinidos rotativos con límite reducido para cuidar RAM
- Generación automática de BSSIDs
- Contador de beacons enviados en tiempo real

### 4. Probe Sniffer
**Objetivo:** Detectar qué redes buscan los dispositivos cercanos.

**Cómo usar:**
1. Clic en **Iniciar**
2. El ESP8266 rotará entre canales automáticamente
3. Acerca dispositivos para capturar sus probes
4. Verás MACs y nombres de redes buscadas
5. Clic en **Detener** al finalizar

**Qué revela:**
- Historial de conexiones del dispositivo
- Redes frecuentadas por el usuario
- Patrones de movimiento (casa, trabajo, etc.)

### 5. Evil Twin
**Objetivo:** Clonar una red WiFi existente.

**Cómo usar:**
1. Clic en **Escanear** (escanea canales 1-13)
2. Selecciona la red a clonar
3. Clic en **Clonar**
4. Se creará un AP con el mismo nombre
5. Opcional: Activa el Portal para capturar credenciales
6. Clic en **Detener** para apagar el clon

**Advertencia:**
- Puede desconectarte del dashboard si cambia el canal
- Usa con precaución en entornos productivos

## Templates Disponibles

| Template | Descripción | Campos |
|----------|-------------|--------|
| Facebook | Login de Facebook con diseño actual | Email + Contraseña |
| Instagram | Login de Instagram minimalista | Usuario + Contraseña |
| Microsoft | Login de Microsoft (Fluent Design) | Email + Contraseña |
| X (Twitter) | Login de X con dark mode | Usuario + Contraseña |
| Google | Login de Google ultra minimalista | Email + Contraseña |
| WiFi Login | Portal genérico para contraseña WiFi | Contraseña WiFi x2 |
| Netflix | Login de Netflix con fondo oscuro | Email + Contraseña |
| WhatsApp | Verificación de número de teléfono | Teléfono + Código |

## Flujo de Uso Típico

### Auditoría Básica
1. **Configurar SSID:** Cambia el nombre a algo creíble para el entorno
2. **Seleccionar Template:** Elige el más apropiado para el contexto
3. **Activar Portal:** Activa el captive portal
4. **Esperar:** Las víctimas se conectan y ven el portal
5. **Capturar:** Las credenciales aparecen en el dashboard
6. **Documentar:** Exporta el reporte de sesión y evita copiar valores crudos fuera del laboratorio

### Auditoría Avanzada
1. **Reconocimiento:** Usa Probe Sniffer para ver redes buscadas
2. **Escaneo:** Usa Deauth Scan para ver todas las redes del área
3. **Selección:** Identifica el objetivo principal
4. **Ataque:** Activa el módulo correspondiente
5. **Documentación:** Registra todos los eventos en los logs
6. **Limpieza:** Detén todos los módulos y borra credenciales

## Ejemplos de Auditoría

### Escenario 1: Oficina Corporativa
- **SSID:** `CorpWiFi-Visitantes`
- **Template:** Microsoft Login
- **Objetivo:** Demostrar riesgo de phishing interno

### Escenario 2: Hotel/Aeropuerto
- **SSID:** `WiFi Gratis Hotel`
- **Template:** WiFi Login
- **Objetivo:** Demostrar riesgo de redes públicas

### Escenario 3: Concientización
- **SSID:** `Free Airport WiFi`
- **Template:** Google
- **Objetivo:** Training de seguridad para empleados

## Configuración Avanzada

### Channel Hopping
El ESP8266 rota automáticamente entre canales 1-13 para:
- Escanear todas las redes del área
- Capturar probes en cualquier canal
- Atacar objetivos en cualquier canal

Configurable en `config.h`:
```cpp
#define CHANNEL_HOPPING_ENABLED true
#define CHANNEL_HOP_INTERVAL 500  // ms por canal
#define MIN_CHANNEL 1
#define MAX_CHANNEL 13
```

### Canal del AP
Por defecto en canal 6. Cambiable en `config.h`:
```cpp
#define AP_CHANNEL 6
```

## Troubleshooting

### El dashboard no carga
- Verifica que estás conectado a la red `PhantomKit`
- Intenta `http://192.168.4.1/dashboard` directamente
- Hard refresh: `Ctrl+Shift+R`
- Revisa el monitor serial para errores

### No aparecen redes en el escaneo
- Espera 10-15 segundos para el escaneo multi-canal
- Verifica que haya redes WiFi en el área
- Revisa los logs para errores de escaneo

### Los ataques no funcionan
- Verifica que el canal coincida con el objetivo
- Asegúrate de que el módulo esté activo
- Revisa el contador de frames/beacons/probes

### Pérdida de conexión al dashboard
- Algunos ataques cambian el canal temporalmente
- El ESP8266 vuelve al canal del AP automáticamente
- Reconecta al WiFi si es necesario
