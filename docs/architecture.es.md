# Arquitectura - ESP8266 PhantomKit

> [English version](architecture.md)

PhantomKit es una plataforma autónoma de concientización en seguridad Wi-Fi para laboratorios controlados y auditorías autorizadas. El ESP8266 ejecuta el access point de gestión, DNS cautivo, dashboard web, API local, templates del portal, capa de almacenamiento y módulos Wi-Fi en un solo dispositivo.

## Objetivos de Diseño

- Funcionar sin infraestructura externa después de flashear
- Mantener el flujo del operador basado en navegador y sin depender de un equipo específico
- Separar los módulos Wi-Fi para iniciar, detener y probar cada función de forma independiente
- Conservar un modo de demo seguro para portafolio con salida redactada en dashboard/API
- Documentar límites legales y técnicos de forma clara para uso responsable

## Vista General del Sistema

```text
Navegador / Dispositivo del operador
        |
        | Dashboard HTTP + API
        v
ESP8266 SoftAP: PhantomKit
        |
        +-- ESP8266WebServer
        |      +-- UI del dashboard
        |      +-- Auth/cookie de sesión
        |      +-- Endpoints de control de módulos
        |      +-- Endpoints de exportación
        |
        +-- DNSServer
        |      +-- Respuestas DNS para captive portal
        |
        +-- LittleFS
        |      +-- Templates HTML del portal
        |      +-- Datos de credenciales/sesión
        |      +-- Configuración de notificaciones
        |
        +-- Módulos Wi-Fi
               +-- Deauth
               +-- Beacon flood
               +-- Probe sniffer
               +-- Evil twin
               +-- Channel hopper
               +-- Auto-portal
               +-- Captura PMKID
```

## Flujo de Arranque

1. `main.cpp` inicializa logs seriales y monta LittleFS.
2. Se cargan credenciales guardadas y configuración de notificaciones.
3. Opcionalmente se inicia Wi-Fi uplink para NTP y webhooks.
4. El AP de gestión arranca con `AP_SSID`, `AP_PASSWORD` y `AP_CHANNEL`.
5. Se inician DNS, channel hopping, módulos Wi-Fi y servidor web del dashboard.
6. El portal cautivo se adjunta al servidor web y sirve templates desde LittleFS.

## Capa de Dashboard y API

`src/wifi/web_server.cpp` contiene la UI del navegador y la API local de control. Maneja:

- Autenticación del dashboard con token de sesión generado y cookie HttpOnly
- Acciones para iniciar/detener módulos
- Polling de estado para contadores del dashboard
- Listado de credenciales con campos redactados cuando `DASHBOARD_REDACT_CREDENTIALS` está activo
- Bloqueo de exportación CSV/reporte mientras el modo seguro de demo está activo
- Exportación PMKID en formato hashcat 22000
- Endpoint de emergency wipe para limpieza de laboratorio

Las tablas dinámicas del dashboard escapan valores antes de insertarlos en HTML para reducir riesgo de XSS por SSIDs, nombres de probes, campos capturados y líneas hash generadas.

## Flujo del Portal Cautivo

1. El ESP8266 ejecuta un SoftAP y un servidor DNS.
2. Las solicitudes DNS se redirigen hacia la dirección local del dashboard/portal.
3. Cuando el portal está activo, las rutas HTTP desconocidas sirven el template seleccionado desde `data/templates/`.
4. Los campos enviados se guardan mediante `CredentialStore`.
5. El dashboard consulta `/api/credentials` y muestra valores redactados por defecto.

## Modelo de Almacenamiento

LittleFS almacena templates del portal y artefactos de ejecución. El credential store mantiene entradas recientes en memoria y agrega filas capturadas a `/credentials.csv`.

Trade-off importante: los datos en LittleFS no están cifrados. El proyecto documenta esta limitación de acceso físico e incluye controles de emergency wipe para limpieza de laboratorio.

## Límites de Módulos Wi-Fi

| Módulo | Responsabilidad |
|---|---|
| `APManager` | Ciclo de vida del SoftAP, cambios de SSID, stealth mode |
| `PhantomDNSServer` | Comportamiento DNS del captive portal |
| `DeauthModule` | Escaneo de objetivos, sniffing de clientes, transmisión de frames deauth |
| `BeaconFloodModule` | Generación de beacons con SSIDs falsos |
| `ProbeSnifferModule` | Captura de probe requests, lookup OUI, soporte para callback Karma |
| `EvilTwinModule` | Escaneo de objetivos y flujo de clonación de AP |
| `AutoPortalModule` | Sugerencia de template por palabras clave del objetivo/entorno |
| `ChannelHopper` | Rotación controlada de canales 1-13 |
| `PMKIDCaptureModule` | Parsing EAPOL/RSN y generación de export hashcat |
| `CredentialStore` | Registros de credenciales/sesión en memoria y LittleFS |
| `NotifierModule` | Notificaciones opcionales por webhook/ntfy |

## Restricciones de Recursos

El ESP8266 tiene RAM limitada y un solo radio Wi-Fi. PhantomKit mantiene una implementación compacta, pero estas restricciones influyen el diseño:

- Algunos módulos compiten por control de canal y modo promiscuo.
- El HTML/JS del dashboard está embebido como strings en memoria de programa para evitar assets externos.
- Las tablas grandes usan PROGMEM cuando es práctico.
- El firmware actual compila cerca del rango alto de RAM, así que cada feature nueva debe medirse con `pio run`.

## Controles de Seguridad y Demo

- La autenticación del dashboard es independiente de la contraseña del AP.
- Las credenciales por defecto están marcadas como `change-me-*`.
- `DASHBOARD_REDACT_CREDENTIALS` redacta campos capturados en API/dashboard.
- Las exportaciones CSV/reporte se desactivan mientras la redacción está activa.
- Emergency wipe elimina artefactos sensibles de LittleFS y reinicia el dispositivo.
- El README, la política SECURITY y los documentos legales incluyen lenguaje de uso responsable.

## Trade-Offs Conocidos

- Los tokens de sesión son pseudoaleatorios, adecuados para laboratorio local pero no autenticación criptográfica fuerte.
- El almacenamiento LittleFS está en texto plano.
- El radio del ESP8266 no permite ejecutar todos los módulos simultáneamente de forma confiable.
- El proyecto está pensado para laboratorios autorizados y concientización, no para pruebas sin permiso.

## Puntos para Portafolio

- Construcción de una herramienta completa de seguridad embebida sobre un microcontrolador con recursos limitados.
- Implementación modular alrededor de frames de gestión 802.11, probe requests y parsing EAPOL.
- Diseño de dashboard web y API local sin depender de servicios cloud.
- Controles de uso responsable para demostrar el proyecto sin exponer datos sensibles reales.
- Balance entre usabilidad, seguridad y restricciones reales del ESP8266.
