# Politica de Seguridad

> [English version](SECURITY.md)

## Versiones soportadas

| Version | Estado |
|---------|--------|
| v1.3.x  | Mantenimiento activo |
| v1.2.x  | Solo correcciones de seguridad |
| < v1.2  | Sin soporte |

---

## Uso responsable

PhantomKit es una herramienta de investigacion de seguridad destinada exclusivamente a:

- Pruebas de penetracion autorizadas en redes propias o con permiso escrito explicito
- Investigacion academica y educativa en entornos de laboratorio controlados
- Competencias Capture-the-Flag (CTF)
- Capacitacion en concientizacion de seguridad para organizaciones

**Usar PhantomKit contra redes sin autorizacion es ilegal en la mayoria de las jurisdicciones** (incluyendo pero no limitandose a la Ley de Fraude y Abuso Informatico en EE.UU., la Ley de Uso Indebido de Computadoras en el Reino Unido, y leyes equivalentes en Mexico, la UE y otros paises). Los autores no aceptan responsabilidad alguna por el uso indebido.

---

## Reportar una vulnerabilidad

Tomamos la seguridad en serio — incluyendo vulnerabilidades en el propio PhantomKit (por ejemplo, bypass de autenticacion del dashboard, path traversal en LittleFS, endpoints sin proteccion).

### Como reportar

**No abras un issue publico en GitHub para vulnerabilidades de seguridad.**

En su lugar, usa uno de los siguientes canales:

1. **GitHub Private Security Advisory** (recomendado)
   Ve a [Security > Advisories > New draft advisory](https://github.com/chrisq-dev/phantom-kit/security/advisories/new) y envia un reporte privado.

2. **Correo electronico**
   Envia un mensaje al mantenedor. Los datos de contacto estan disponibles en el perfil de GitHub del mantenedor.

### Que incluir

Un buen reporte de vulnerabilidad incluye:

- **Descripcion** — que es la vulnerabilidad y donde aparece en el codigo
- **Impacto** — que podria lograr un atacante (por ejemplo, secuestro de sesion, robo de credenciales, ejecucion remota de codigo)
- **Pasos para reproducir** — pasos minimos de reproduccion en un NodeMCU v2
- **Correccion sugerida** (opcional pero apreciada)

### Tiempos de respuesta

| Etapa | Tiempo objetivo |
|-------|-----------------|
| Confirmacion | 48 horas |
| Evaluacion inicial | 7 dias |
| Correccion o mitigacion | 30 dias |
| Divulgacion publica | Despues de publicar la correccion |

Seguimos un modelo de **divulgacion coordinada**. Si reportas una vulnerabilidad, te daremos credito en las notas de la version y el CHANGELOG, a menos que prefieras permanecer anonimo.

---

## Limitaciones de seguridad conocidas

PhantomKit es firmware embebido que corre en un dispositivo con recursos limitados. Las siguientes son decisiones de diseno conocidas, no bugs:

| Limitacion | Notas |
|---|---|
| `DASHBOARD_PASSWORD` se almacena en texto plano en `config.h` | Cambiar antes de flashear. No subir `config.h` con credenciales reales a repositorios publicos. |
| Los tokens de sesion se generan con `random()` (no es un CSPRNG) | El ESP8266 no tiene RNG por hardware. Los tokens son suficientes para autenticacion en red local, pero no son criptograficamente seguros. |
| `/api/panic` no requiere autenticacion | Intencional — permite borrado de emergencia cuando no se puede acceder al dashboard. Deshabilitar en `config.h` si no es necesario. |
| Los campos de credenciales del dashboard/API se redactan por defecto | `DASHBOARD_REDACT_CREDENTIALS` esta activo para demos y screenshots de portafolio. Desactivarlo solo en un laboratorio autorizado cuando se necesiten valores reales. |
| Las credenciales en LittleFS se almacenan sin cifrar | El acceso fisico al dispositivo permite extraer credenciales. Usar Emergency Wipe si el dispositivo puede estar comprometido. |
| Las URLs de webhook se almacenan en texto plano en LittleFS | Tratar el dispositivo como cualquier otro dispositivo que contenga credenciales. |

---

## Alcance

Lo siguiente esta **dentro del alcance** para reportes de vulnerabilidades:

- Bypass de autenticacion del dashboard
- Ejecucion remota de codigo via la interfaz web
- Path traversal o lectura/escritura arbitraria de archivos en LittleFS
- Robo de credenciales del dashboard sin autenticacion
- CSRF en endpoints sensibles

Lo siguiente esta **fuera del alcance**:

- Ataques que requieren acceso fisico al dispositivo (se asume hostil)
- Denegacion de servicio contra el propio ESP8266 (es un microcontrolador)
- Problemas en bibliotecas externas (framework Arduino para ESP8266, LittleFS) — reportarlos aguas arriba
- El hecho de que la herramienta pueda usarse con fines ofensivos — eso es por diseno para uso autorizado
