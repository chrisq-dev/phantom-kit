# Demo Guiado para Portafolio

> [English version](demo.md)

Este guion sirve para presentar PhantomKit en portafolio aunque no tengas un ESP8266 a la mano. Explica qué hace el proyecto, cómo fluye la operación y qué controles lo hacen presentable como herramienta de entrenamiento autorizado.

## Pitch de 60 Segundos

PhantomKit es una plataforma autónoma de concientización Wi-Fi para ESP8266. El dispositivo ejecuta su propio AP de gestión, DNS cautivo, dashboard web, templates de portal, almacenamiento local y módulos Wi-Fi sin servidor externo en laptop. El proyecto demuestra seguridad embebida, manejo de protocolos Wi-Fi, diseño bajo límites reales de memoria y controles responsables como autenticación del dashboard, bloqueo de credenciales default, modo pasivo, salida redactada para demos, emergency wipe y checksums de releases.

## Qué Mostrar Sin Hardware

Usa el repositorio como artefacto principal del demo:

1. Abre `README.es.md` y explica el diagrama de arquitectura, threat model y assets de release.
2. Abre `docs/safety.es.md` y resalta el perfil pasivo, redacción, hardening de auth y emergency wipe.
3. Abre `src/wifi/web_server.cpp` y señala:
   - `/api/audit` para el wizard de auditoría.
   - `/api/passive` para activar/desactivar el modo pasivo.
   - `rejectIfPassive()` para bloquear módulos activos.
   - `handleAPIExportReport()` para reportes compatibles con redacción.
4. Abre `.github/workflows/build.yml` y explica que los tags generan firmware, LittleFS y checksums.
5. Muestra el release más reciente en GitHub y su `SHA256SUMS.txt`.

Con eso puedes demostrar diseño y criterio técnico sin simular que el hardware está corriendo en vivo.

## Guion de Dashboard

Usa estos puntos con screenshots más adelante, o como walkthrough narrado del código:

1. **Login y setup:** El dashboard usa una contraseña separada del AP y rechaza el acceso si siguen compiladas las credenciales `change-me-*`.
2. **Wizard de auditoría:** El operador selecciona `passive`, `portal` o `full` y registra alcance con SSID/BSSID/canal.
3. **Modo pasivo:** En modo pasivo, PhantomKit bloquea deauth, beacon flood, evil twin, auto-attack, inicio de portal y karma. Siguen disponibles escaneos, probe sniffer, captura PMKID, logs, reportes y wipe.
4. **Flujo de portal:** Para concientización autorizada, el operador elige template, activa portal, usa valores de prueba y exporta un reporte de sesión.
5. **Reporte:** El reporte incluye alcance, estado de modo pasivo, contadores, hallazgos, estado de datos capturados y log operativo. Con redacción activa, omite campos crudos.
6. **Limpieza:** El operador ejecuta wipe desde dashboard o GPIO antes de guardar o entregar el dispositivo.

## Explicación para Portafolio

Puedes usar este párrafo en README, sitio personal o entrevista:

> PhantomKit no se presenta como un gadget ofensivo genérico. Está planteado como plataforma de laboratorio y concientización autorizada. El valor técnico está en la implementación ESP8266: un dispositivo limitado, de radio único, ejecutando dashboard local, DNS cautivo, templates LittleFS, módulos de frames Wi-Fi, parsing PMKID, reportes y controles de seguridad. El valor profesional está en el threat model, modo pasivo, demos redactados, checksums de release y documentación que explica dónde aplica la herramienta y dónde no.

## Notas de Issues / Roadmap

Los issues abiertos quedan como trabajo futuro real:

- `#4 PMKID`: el firmware actual captura/exporta líneas hashcat PMKID; persistencia a LittleFS/SD queda pendiente.
- `#3 SD card`: sigue abierto porque falta autodetección SD y espejo de logs/credenciales.
- `#2 Webhooks`: el firmware actual soporta ntfy/webhook HTTP configurable; retry, toggle y payloads específicos para Discord/Slack quedan pendientes.

Mantenerlos abiertos es mejor que cerrarlos antes de cumplir todos los criterios porque demuestra criterios de aceptación honestos.
