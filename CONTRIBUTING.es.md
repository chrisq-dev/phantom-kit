# Contribuir a PhantomKit

Gracias por tomarte el tiempo de contribuir. PhantomKit es una plataforma de auditoria Wi-Fi de codigo abierto para ESP8266 y da la bienvenida a contribuciones de la comunidad de investigacion de seguridad y makers.

**Aviso legal:** Todas las contribuciones deben estar destinadas a uso en auditorias de seguridad autorizadas, investigacion o entornos educativos. Consulta [SECURITY.es.md](SECURITY.es.md) para la politica de uso responsable.

> [English version](CONTRIBUTING.md)

---

## Tabla de contenidos

- [Primeros pasos](#primeros-pasos)
- [Entorno de desarrollo](#entorno-de-desarrollo)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Reportar issues](#reportar-issues)
- [Enviar Pull Requests](#enviar-pull-requests)
- [Estilo de codigo](#estilo-de-codigo)
- [Formato de commits](#formato-de-commits)
- [Pruebas en hardware](#pruebas-en-hardware)

---

## Primeros pasos

1. Haz un **fork** del repositorio en GitHub
2. **Clona** tu fork localmente:
   ```bash
   git clone https://github.com/TU_USUARIO/phantom-kit.git
   cd phantom-kit
   ```
3. Crea una **rama de feature**:
   ```bash
   git checkout -b feat/mi-nueva-funcionalidad
   ```
4. Realiza tus cambios, prueba en hardware y abre un PR.

---

## Entorno de desarrollo

### Requisitos

| Herramienta | Version | Instalacion |
|-------------|---------|-------------|
| PlatformIO Core | >= 6.x | `pip install platformio` |
| PlatformIO IDE | cualquiera | [Extension VS Code](https://platformio.org/install/ide?install=vscode) |
| `gh` CLI | cualquiera | [cli.github.com](https://cli.github.com) |
| Placa ESP8266 | NodeMCU v2 / ESP-12E | — |

### Primera compilacion

```bash
# Compilar el firmware
pio run

# Flashear el sistema de archivos (templates del portal)
pio run --target uploadfs

# Flashear el firmware
pio run --target upload

# Monitor serie
pio device monitor --baud 115200
```

### Configuracion

Personaliza los ajustes de compilacion en `src/config.h`:

```cpp
#define AP_SSID            "PhantomKit"
#define AP_PASSWORD        "change-me-phantomkit"
#define DASHBOARD_PASSWORD "change-me-auditor"
#define UPLINK_SSID        ""          // Opcional — activa NTP y webhooks
#define UPLINK_PASSWORD    ""
```

---

## Estructura del proyecto

```
phantom-kit/
├── src/
│   ├── main.cpp                    # Setup, loop, instancias globales
│   ├── config.h                    # Toda la configuracion en tiempo de compilacion
│   ├── notifier.h / .cpp           # Modulo de notificaciones webhook
│   ├── portal/
│   │   ├── captive_portal.h/.cpp   # Logica del portal cautivo
│   │   └── credential_store.h/.cpp # Persistencia de credenciales en LittleFS
│   └── wifi/
│       ├── ap_manager.h/.cpp       # Gestion del SoftAP (stealth, SSID)
│       ├── auto_portal.h/.cpp      # Auto-match de SSID / sugerencia de template
│       ├── beacon_flood.h/.cpp     # Ataque beacon flood
│       ├── channel_hopper.h/.cpp   # Salto de canal
│       ├── deauth.h/.cpp           # Ataque deauth (broadcast + unicast)
│       ├── dns_server.h/.cpp       # DNS del portal cautivo
│       ├── evil_twin.h/.cpp        # AP gemelo malicioso
│       ├── oui.h                   # Lookup de fabricante OUI (PROGMEM)
│       ├── pmkid_capture.h/.cpp    # Captura de PMKID WPA2
│       ├── probe_sniffer.h/.cpp    # Sniffer de probe requests 802.11 + Karma
│       └── web_server.h/.cpp       # Servidor HTTP del dashboard
├── data/
│   └── templates/                  # Templates HTML del portal (LittleFS)
├── .github/
│   ├── workflows/build.yml         # CI — compilacion en cada push
│   └── ISSUE_TEMPLATE/             # Plantillas de issues
├── platformio.ini
├── CHANGELOG.md / CHANGELOG.es.md
├── CONTRIBUTING.md / CONTRIBUTING.es.md
├── SECURITY.md / SECURITY.es.md
└── CODE_OF_CONDUCT.md / CODE_OF_CONDUCT.es.md
```

---

## Reportar issues

Antes de abrir un issue:

1. Busca en los issues existentes para evitar duplicados.
2. Revisa el FAQ en el README.
3. Usa la plantilla de issue correspondiente (reporte de bug o solicitud de feature).

Un buen reporte de bug incluye:
- Modelo de placa (NodeMCU v2, Wemos D1 Mini, etc.)
- Version de PlatformIO (`pio --version`)
- Salida completa del monitor serie
- Pasos para reproducir el problema
- Comportamiento esperado vs comportamiento real

---

## Enviar Pull Requests

### Que aceptamos

- Correcciones de bugs
- Nuevos modulos de ataque o templates de portal
- Soporte para placas ESP8266/ESP32 adicionales
- Mejoras en la documentacion
- Traducciones
- Utilidades de prueba (scripts de laboratorio, herramientas de analisis de capturas)

### Lista de verificacion para PRs

- [ ] Probado en hardware fisico (no solo compilado)
- [ ] `pio run` se completa sin errores
- [ ] El uso de RAM se mantiene por debajo del 90% (mostrado en la salida de `pio run`)
- [ ] Las nuevas funcionalidades estan documentadas en un bloque de comentarios o en la seccion correspondiente del README
- [ ] Los mensajes de commit siguen el formato [Conventional Commits](#formato-de-commits)
- [ ] La descripcion del PR explica *por que* se necesita el cambio, no solo *que* hace

### Mantener los PRs enfocados

Un PR = un feature o corrección. Los PRs grandes que tocan muchas areas no relacionadas seran solicitados para dividirse.

---

## Estilo de codigo

PhantomKit esta escrito en C++ (framework Arduino). Sigue estas convenciones:

### Nomenclatura

```cpp
// Clases: PascalCase
class ProbeSnifferModule { ... };

// Metodos y variables: camelCase
void startSniffing(int channel);
bool isRunning;

// Constantes y macros: UPPER_SNAKE_CASE
#define AP_CHANNEL 6
#define MAX_CREDENTIALS 50

// Archivos: snake_case
probe_sniffer.cpp
```

### Reglas de memoria en ESP8266

El ESP8266 tiene 80 KB de RAM. Ten esto en cuenta:

- Guarda tablas de constantes grandes en `PROGMEM` (ver `oui.h` como ejemplo)
- Usa `FPSTR()` para literales de texto largos servidos por HTTP
- Evita asignaciones dinamicas en rutas criticas (`new` / `malloc`)
- Los callbacks en modo promiscuo **deben** ser `IRAM_ATTR`
- Verifica el uso de RAM tras cada cambio significativo: objetivo por debajo del 90%

### Patron de modulos

Todos los modulos siguen este patron:

```cpp
class MiModulo {
public:
    MiModulo();
    void begin();        // Inicializar, registrar mensaje de arranque
    void update();       // Llamado en cada iteracion de loop()
    bool isRunning() const;
private:
    bool running;
};
```

---

## Formato de commits

PhantomKit usa [Conventional Commits](https://www.conventionalcommits.org/):

```
<tipo>(<ambito>): <descripcion corta>

[cuerpo opcional]
[pie opcional]
```

### Tipos

| Tipo | Cuando usarlo |
|------|---------------|
| `feat` | Nuevo feature o modulo de ataque |
| `fix` | Correccion de bug |
| `docs` | Solo documentacion |
| `refactor` | Reestructuracion de codigo sin cambio de comportamiento |
| `perf` | Mejora de rendimiento |
| `chore` | Proceso de build, dependencias, CI |
| `test` | Agregar utilidades de prueba |

### Ejemplos

```
feat(probe): add OUI manufacturer lookup from PROGMEM table
fix(deauth): stop promiscuous callback before channel switch
docs(readme): add wiring diagram for NodeMCU v2
chore(ci): pin PlatformIO version to 6.1.11
```

---

## Pruebas en hardware

PhantomKit no tiene pruebas unitarias automatizadas — el firmware para targets embebidos es dificil de simular. Todas las pruebas son manuales y **deben realizarse en un entorno de laboratorio controlado**.

### Configuracion de laboratorio recomendada

```
[Laptop (monitor)]  --USB-->  [NodeMCU (PhantomKit)]  --WiFi-->  [Telefono de prueba (victima)]
```

- Usa un telefono de prueba dedicado o una maquina virtual con adaptador WiFi
- Nunca realices pruebas en redes que no sean tuyas o para las que no tengas autorizacion escrita explicita
- Para pruebas de PMKID, usa un router propio que controles
- Para pruebas de Evil Twin / Karma, usa el hotspot de tu propio telefono

### Verificar una compilacion

```bash
pio run                          # Debe completarse con < 90% de RAM
pio run --target uploadfs        # Flashear templates
pio run --target upload          # Flashear firmware
pio device monitor --baud 115200 # Verificar log de arranque
```

Salida esperada en el arranque:
```
[T+00:00:01] ESP8266 PhantomKit iniciando...
[T+00:00:01] Version 1.3.0
[T+00:00:02] WiFi AP iniciado: PhantomKit Canal: 6
[T+00:00:02] Dashboard web: http://192.168.4.1/dashboard
[T+00:00:02] PhantomKit listo.
```

---

## Preguntas

Abre una [Discusion](https://github.com/chrisq-dev/phantom-kit/discussions) o comunicate a traves del issue tracker.
