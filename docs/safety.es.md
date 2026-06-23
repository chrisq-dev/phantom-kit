# Seguridad por Diseño

> [English version](safety.md)

PhantomKit incluye funciones de simulación Wi-Fi que pueden interrumpir redes o recolectar entradas sensibles si se usan mal. El proyecto está diseñado únicamente para laboratorios propios, capacitación de concientización y auditorías con alcance escrito. Esta página documenta los controles que permiten demostrar la herramienta sin normalizar una operación insegura.

## Objetivos de Seguridad

- Hacer explícito el uso autorizado en la documentación y superficie del producto.
- Mantener la salida de demos redactada por defecto.
- Evitar despliegues accidentales con credenciales públicas de gestión.
- Mantener limpieza destructiva disponible, pero detrás de autenticación del dashboard.
- Favorecer memoria limitada, recolección limitada y trade-offs claros para el operador.

## Controles Implementados

| Control | Implementación | Propósito |
|---|---|---|
| Bloqueo de credenciales default | `REQUIRE_CUSTOM_CREDENTIALS` bloquea el dashboard si siguen compilados los valores `change-me-*` | Evita flasheos casuales con defaults públicos |
| Ruta separada para dashboard | El dashboard usa `/dashboard/login`; el portal cautivo usa `/login` | Evita mezclar autenticación del auditor con entrada del portal |
| Bloqueo de intentos local | `LOGIN_MAX_ATTEMPTS` + `LOGIN_LOCKOUT_MS` | Reduce riesgo de brute force local |
| Cookie de sesión endurecida | Cookie HttpOnly con `SameSite=Strict` y token regenerado tras login | Reduce exposición accidental de sesión |
| Modo pasivo de auditoría | El wizard habilita un perfil que bloquea deauth, beacon flood, evil twin, auto-attack, inicio de portal y karma | Permite observación y reportes sin disrupción RF activa |
| Modo demo redactado | `DASHBOARD_REDACT_CREDENTIALS` oculta valores crudos y desactiva exports | Hace seguras capturas de pantalla y demos públicas |
| Reportes compatibles con redacción | Los reportes incluyen alcance, contadores, hallazgos y logs mientras omiten campos crudos si la redacción está activa | Permite evidencia profesional sin filtrar secretos |
| Redacción en Serial | `SERIAL_REDACT_CREDENTIALS` oculta campos crudos en logs seriales | Evita filtraciones por monitor serial |
| Emergency wipe autenticado | `/api/panic` requiere sesión del dashboard | Evita borrado no autenticado por cualquier cliente del AP |
| Buffers acotados | Defaults más pequeños para probes, PMKIDs, targets y credenciales | Reduce presión de memoria y retención innecesaria |
| Escape CSV/JSON | Valores de export/status se escapan antes de serializar | Evita exports malformados y fallos del dashboard |

## Checklist del Operador

Antes de cualquier demo o auditoría:

1. Confirmar autorización escrita y alcance.
2. Cambiar `AP_PASSWORD` y `DASHBOARD_PASSWORD`.
3. Usar el perfil pasivo salvo que el alcance escrito autorice módulos activos.
4. Mantener `DASHBOARD_REDACT_CREDENTIALS=1` para screenshots, portafolio y demos públicas.
5. Usar credenciales falsas o de entrenamiento cuando sea posible.
6. Detener módulos activos al terminar el ejercicio.
7. Ejecutar emergency wipe o wipe por GPIO antes de guardar, compartir o reutilizar el dispositivo.

## Manejo de Datos

Las filas capturadas se almacenan en LittleFS y no están cifradas. Trata el dispositivo como sensible después de cualquier ejercicio. El proyecto soporta limpieza rápida desde dashboard y GPIO, pero el modelo operativo más seguro es evitar recolectar secretos reales.

## Seguridad de Releases

Los assets oficiales se generan desde GitHub Actions al publicar tags e incluyen checksums SHA-256. Esto mantiene los builds revisables y evita distribuir firmware local sin trazabilidad.

## Riesgo Residual

- ESP8266 no ofrece almacenamiento moderno de secretos respaldado por hardware.
- HTTP local en el AP de gestión no usa TLS.
- Los módulos Wi-Fi pueden interrumpir redes cercanas si se usan fuera de alcance.
- Acceso físico al dispositivo puede exponer contenido de LittleFS.

Estas limitaciones se documentan intencionalmente porque el proyecto debe demostrar criterio además de capacidad técnica.
