# Safety by Design

> [Versión en español](safety.es.md)

PhantomKit includes Wi-Fi security simulation features that can disrupt networks or collect sensitive input if misused. The project is designed for owned labs, security awareness training, and written-scope audits only. This page documents the controls that make the tool demonstrable without normalizing unsafe operation.

## Safety Goals

- Make authorized use explicit in the product surface and documentation.
- Keep demo output redacted by default.
- Prevent accidental deployment with default management credentials.
- Keep destructive cleanup available, but behind dashboard authentication.
- Favor bounded memory, bounded collection, and clear operator trade-offs.

## Controls Implemented

| Control | Implementation | Purpose |
|---|---|---|
| Default credential guard | `REQUIRE_CUSTOM_CREDENTIALS` blocks dashboard login while `change-me-*` values are compiled in | Prevents casual flashing with public defaults |
| Separate dashboard route | Dashboard auth uses `/dashboard/login`; captive portal submissions use `/login` | Avoids confusing operator auth with portal input |
| Local login lockout | `LOGIN_MAX_ATTEMPTS` + `LOGIN_LOCKOUT_MS` | Reduces local brute-force risk |
| Session cookie hardening | HttpOnly cookie with `SameSite=Strict` and regenerated token after login | Reduces accidental session exposure |
| Redacted demo mode | `DASHBOARD_REDACT_CREDENTIALS` blocks raw dashboard/API values and disables exports | Keeps portfolio screenshots and demos safe |
| Serial redaction | `SERIAL_REDACT_CREDENTIALS` hides raw captured fields in serial logs | Prevents raw secrets leaking through serial monitors |
| Authenticated emergency wipe | `/api/panic` requires dashboard session | Prevents unauthenticated data deletion by any AP client |
| Bounded capture buffers | Smaller defaults for probes, PMKIDs, targets, and credentials | Reduces memory pressure and limits unnecessary retention |
| CSV/JSON escaping | Export/status values are escaped before serialization | Prevents malformed exports and dashboard breakage |

## Operator Checklist

Before any demo or audit:

1. Confirm written authorization and scope.
2. Change `AP_PASSWORD` and `DASHBOARD_PASSWORD`.
3. Keep `DASHBOARD_REDACT_CREDENTIALS=1` for screenshots, portfolio material, and public demos.
4. Use fake or training credentials whenever possible.
5. Stop active modules when the exercise ends.
6. Run emergency wipe or hardware wipe before storing, sharing, or reusing the device.

## Data Handling

Captured rows are stored in LittleFS and are not encrypted. Treat the device as sensitive after any exercise. The project intentionally supports fast cleanup through dashboard wipe and GPIO wipe, but the safest operational model is to avoid collecting real secrets in the first place.

## Release Safety

Official release assets are produced by GitHub Actions from tags and include SHA-256 checksums. This keeps builds reproducible enough for review and avoids distributing untracked local firmware binaries.

## Remaining Risk

- ESP8266 cannot provide modern hardware-backed secret storage.
- Local HTTP is not TLS-protected on the management AP.
- Wi-Fi modules can disrupt nearby networks if used outside scope.
- Physical access to the device can expose LittleFS contents.

These limitations are documented intentionally because the project is meant to show judgment as much as technical capability.
