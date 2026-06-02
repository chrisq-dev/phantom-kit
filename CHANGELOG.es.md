# Registro de Cambios

Todos los cambios relevantes de ESP8266 PhantomKit están documentados en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
y este proyecto sigue [Versionado Semántico](https://semver.org/spec/v2.0.0.html).

> [English version](CHANGELOG.md)

---

## [1.3.0] - 2026-06-02

### Agregado

#### Auto-Attack Chain
- Nueva pestaña **Auto-Attack** en el dashboard — un clic encadena escaneo, clonación de SSID, deautenticación unicast y activación del portal con el template más adecuado
- Endpoint `POST /api/autoattack` con `action=start|stop|suggest`
- `suggestTemplate(ssid)` clasifica cualquier SSID por palabras clave para seleccionar automáticamente el portal más convincente (ISP, corporativo, redes sociales, streaming)
- `getTemplateNameFor(ssid)` devuelve el nombre legible del template para un SSID dado

#### Ataque Karma
- `setKarmaMode(bool, KarmaCallback)` en `ProbeSnifferModule` — cuando está activo, dispara un callback por cada SSID único detectado en probe requests (excepto broadcast)
- `karmaCallback()` en `main.cpp` cambia el SSID del softAP para coincidir con el SSID buscado y activa el portal automáticamente
- Control desde la pestaña **Ajustes** mediante `POST /api/karma` (`action=start|stop`)
- El dashboard muestra el SSID karma activo en tiempo real mediante el poll de estado

#### Captura de PMKID
- Nuevo `PMKIDCaptureModule` — analiza frames EAPOL Key en modo promiscuo para extraer PMKIDs de WPA2 del primer mensaje del 4-way handshake (sin necesidad de asociación de cliente; Jens Steube, 2018)
- Parser completo de RSN IE: localiza el campo PMKID Count recorriendo las suites de cifrado y AKM
- Deduplicación — el mismo PMKID no se almacena dos veces en una sesión
- `getHashcatOutput()` devuelve todos los PMKIDs capturados en formato hashcat 22000 (`WPA*02*...`)
- Nueva pestaña **PMKID** en el dashboard con controles de inicio/parada y tabla de resultados en vivo
- `GET /api/pmkid/export` — descarga el archivo `.hc22000` directamente desde el dispositivo

#### Lookup de Fabricante OUI
- Nuevo `oui.h` — tabla de lookup en PROGMEM con prefijos OUI de Apple, Samsung, Google, Xiaomi, Huawei, Intel, OnePlus, Motorola, ASUS, LG, Sony, MediaTek y Realtek
- `lookupOUI(mac[3])` devuelve el nombre del fabricante a partir de los primeros 3 bytes de una MAC
- Campo `ProbeDevice.vendor` poblado en la primera detección dentro de `addDevice()`
- Columna de fabricante agregada a la tabla de dispositivos del probe sniffer

#### Emergency Wipe
- `GPIO_WIPE_PIN` (GPIO0 / botón FLASH del NodeMCU) — mantener presionado `GPIO_WIPE_HOLD_MS` (por defecto 3 s) para borrar todos los datos y reiniciar
- `POST /api/panic` — endpoint web sin autenticación (por diseño) que elimina `/credentials.csv` y `/notify.cfg` y luego reinicia
- Botón Emergency Wipe en la pestaña **Ajustes** con diálogo de confirmación

#### Notificación Toast de Credenciales
- El JavaScript del dashboard detecta aumentos en el conteo de credenciales en cada poll de estado (cada 3 segundos)
- `showToast(msg)` — notificación flotante que aparece desde la esquina superior derecha al capturar una nueva credencial; se cierra automáticamente tras 5 segundos
- Funciona en cualquier pestaña activa del dashboard

#### Botón Quick-Attack
- Cada fila de la tabla de dispositivos del probe sniffer tiene un botón **Atacar**
- Al pulsarlo, completa automáticamente el formulario de Auto-Attack con el primer SSID buscado y la MAC del dispositivo, y cambia a la pestaña Auto-Attack

### Modificado
- `ProbeSnifferModule::getDevicesJSON()` ahora incluye el campo `vendor` en cada objeto de dispositivo
- El struct `ProbeDevice` agrega el campo `vendor: String`
- El `setInterval` del dashboard actualiza SSID karma, conteo de PMKID y conteo de dispositivos en un único poll de estado
- El constructor de `PhantomWebServer` ahora requiere un parámetro `PMKIDCaptureModule&`
- `suggestTemplate()` y `getTemplateNameFor()` son ahora métodos `static` públicos de `AutoPortalModule`
- Cadena de versión actualizada a `1.3.0`

### Eliminado
- `scripts/release.sh` — script obsoleto de automatización del release v1.0.0

---

## [1.2.0] - 2026-06-02

### Agregado

#### Autenticación del Dashboard
- Autenticación por token de sesión para todas las rutas `/dashboard` y `/api/*`
- `generateSessionToken()` genera un token hexadecimal aleatorio en el arranque
- `POST /login` — valida `DASHBOARD_PASSWORD` (configurable en `config.h`) y establece una cookie `pk_session` con `HttpOnly`
- `GET /logout` — elimina la cookie de sesión
- Todos los endpoints de API sensibles redirigen a `/login` si no hay sesión activa
- Botón de cerrar sesión en el encabezado del dashboard

#### Probe Sniffer Real
- Reescritura completa de `probe_sniffer.cpp` con análisis real de frames de gestión 802.11
- El callback `IRAM_ATTR probePromiscCb()` extrae la MAC origen (bytes 46–51 del frame), el elemento de información SSID (offset 60+) y el RSSI del encabezado `RxControl` de 36 bytes
- `addDevice()` deduplica por MAC y por lista de SSIDs por dispositivo
- El dashboard muestra tabla de dispositivos: MAC, SSIDs buscados, RSSI
- `GET /api/probe/devices` devuelve JSON de dispositivos en tiempo real

#### Deauth Unicast
- `scanClients()` en `DeauthModule` — escucha pasiva de 800 ms en modo promiscuo antes de cada ataque para detectar MACs de clientes asociados
- `IRAM_ATTR deauthSniffCb()` identifica MACs de clientes a partir de frames de datos 802.11 (bits ToDS/FromDS)
- `update()` envía frames de deautenticación unicast AP→Cliente y Cliente→AP para cada cliente detectado, además del fallback broadcast

#### Notificaciones Webhook
- Nuevo `src/notifier.h/.cpp` — envía HTTP POST en cada captura de credenciales
- Compatible con ntfy.sh (cuerpo de texto plano con ruta de topic) y webhooks JSON genéricos
- Configuración guardada en `/notify.cfg` en LittleFS, persiste entre reinicios
- `POST /api/notify` con `action=save|test`
- Pestaña **Ajustes** en el dashboard: campos de URL y topic con botones Guardar y Probar

#### Timestamps NTP
- `configTime()` se llama en el arranque cuando `UPLINK_SSID` está configurado — sincroniza con `pool.ntp.org`
- `getTimestamp()` devuelve `YYYY-MM-DD HH:MM:SS` cuando está sincronizado, y `T+HH:MM:SS` (tiempo desde el arranque) como fallback

#### Modo Stealth
- `ap_manager.setStealthMode(bool)` usa el SDK `wifi_softap_set_config` para establecer `ssid_hidden = 1`
- `POST /api/stealth` — activar/desactivar sin reflashear
- Estado mostrado y controlado desde la pestaña **Ajustes**

#### CI con GitHub Actions
- `.github/workflows/build.yml` — compila el firmware en cada push y PR a `main`
- Badge de compilación agregado a los READMEs

### Modificado
- Modo WiFi cambiado a `WIFI_AP_STA` cuando `UPLINK_SSID` está configurado
- `ICACHE_RAM_ATTR` reemplazado por `IRAM_ATTR` en todos los callbacks promiscuos (macro obsoleto)
- Buffers de `snprintf` para timestamps ampliados a 32 bytes

### Agregado (config.h)
- `DASHBOARD_PASSWORD` — contraseña de acceso al dashboard
- `UPLINK_SSID` / `UPLINK_PASSWORD` — uplink STA opcional para NTP y webhooks
- `NTP_SERVER`, `NTP_OFFSET`, `NTP_DST` — configuración NTP

---

## [1.1.0] - 2026-06-02

### Agregado
- **Almacenamiento en LittleFS** — persistencia de credenciales capturadas en `/credentials.csv`. Las credenciales sobreviven reinicios y cortes de energía.
- **Exportacion CSV y Reporte** — descarga de credenciales desde el dashboard como `.csv` o reporte de sesion formateado (`phantomkit_report.txt`)
- **Mejoras en templates del portal:**
  - **Facebook** — paso de verificacion de contrasena, animacion de carga, favicon personalizado
  - **Microsoft** — flujo de dos pasos (correo y luego contrasena) con avatar dinamico
  - **WiFi Login** — rediseniado como portal cautivo corporativo estilo Cisco/Aruba con indicador de progreso

---

## [1.0.0] - 2026-06-01

### Lanzamiento inicial

Primera version estable de ESP8266 PhantomKit — plataforma autonoma de auditoria Wi-Fi para el microcontrolador ESP8266.

### Agregado

#### Plataforma base
- Operacion autonoma — sin infraestructura externa requerida
- Dashboard web en `http://192.168.4.1/dashboard` con tabla de credenciales en tiempo real y logs del sistema
- Sistema de archivos LittleFS para servir templates HTML desde flash
- Portal cautivo con redireccion DNS automatica
- Salto de canal automatico (canales 1 al 13)

#### Modulos de ataque
- **Evil Portal** — 8 templates de inicio de sesion clonados (Facebook, Instagram, Microsoft, X, Google, WiFi Login, Netflix, WhatsApp)
- **Deauth Attack** — frames de deautenticacion IEEE 802.11
- **Beacon Flood** — mas de 50 SSIDs falsos
- **Probe Sniffer** — captura pasiva de probe requests
- **Evil Twin** — clonacion de SSID/BSSID
- **Auto-Portal** — escaneo del entorno y recomendacion de template

#### Dashboard
- Tabla de credenciales en tiempo real (template, campos, timestamp)
- Log de eventos en vivo
- Panel de control de ataques (inicio/parada por modulo)
- Configuracion de SSID y template sin necesidad de reflashear

---

[1.3.0]: https://github.com/chrisq-dev/phantom-kit/compare/v1.2.0...v1.3.0
[1.2.0]: https://github.com/chrisq-dev/phantom-kit/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/chrisq-dev/phantom-kit/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/chrisq-dev/phantom-kit/releases/tag/v1.0.0
