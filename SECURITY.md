# Security Policy

## Supported Versions

| Version | Status |
|---------|--------|
| v1.3.x  | Actively maintained |
| v1.2.x  | Security fixes only |
| < v1.2  | Not supported |

---

## Responsible Use

PhantomKit is a security research tool intended exclusively for:

- Authorized penetration testing of networks you own or have explicit written permission to audit
- Academic and educational research in controlled lab environments
- Capture-the-flag (CTF) competitions
- Security awareness training for organizations

**Using PhantomKit against networks without authorization is illegal in most jurisdictions** (including but not limited to the Computer Fraud and Abuse Act in the US, the Computer Misuse Act in the UK, and equivalent laws in Mexico, the EU, and elsewhere). The authors accept no liability for misuse.

---

## Reporting a Vulnerability

We take security seriously — including vulnerabilities in PhantomKit itself (e.g., dashboard auth bypass, LittleFS path traversal, unprotected endpoints).

### How to report

**Do not open a public GitHub issue for security vulnerabilities.**

Instead, use one of the following channels:

1. **GitHub Private Security Advisory** (preferred)
   Go to [Security > Advisories > New draft advisory](https://github.com/chrisq-dev/phantom-kit/security/advisories/new) and submit a private report.

2. **Email**
   Send a message to the maintainer. Contact details are available on the maintainer's GitHub profile.

### What to include

A good vulnerability report includes:

- **Description** — what the vulnerability is and where it appears in the code
- **Impact** — what an attacker could achieve (e.g., session hijack, credential theft, remote code execution)
- **Steps to reproduce** — minimal reproduction steps on a NodeMCU v2
- **Suggested fix** (optional but appreciated)

### Response timeline

| Stage | Target time |
|-------|-------------|
| Acknowledgement | 48 hours |
| Initial assessment | 7 days |
| Fix or mitigation | 30 days |
| Public disclosure | After fix is released |

We follow a **coordinated disclosure** model. If you report a vulnerability, we will credit you in the release notes and CHANGELOG unless you prefer to remain anonymous.

---

## Known Security Limitations

PhantomKit is embedded firmware running on a resource-constrained device. The following are known design trade-offs, not bugs:

| Limitation | Notes |
|---|---|
| `DASHBOARD_PASSWORD` is stored in plaintext in `config.h` | Change before flashing. Do not commit `config.h` with real credentials to public repositories. |
| Session tokens are generated with `random()` (not a CSPRNG) | The ESP8266 has no hardware RNG. Tokens are sufficient for local-network auth but not cryptographically secure. |
| `/api/panic` has no authentication | Intentional — allows emergency wipe when dashboard auth is unavailable. Disable in `config.h` if not needed. |
| LittleFS credentials are stored unencrypted | Physical access to the device allows credential extraction. Use emergency wipe if the device may be compromised. |
| Webhook URLs are stored in plaintext on LittleFS | Treat the device as you would any other credential-bearing device. |

---

## Scope

The following are **in scope** for vulnerability reports:

- Dashboard authentication bypass
- Remote code execution via the web interface
- Path traversal or arbitrary file read/write on LittleFS
- Credential theft from the dashboard without authentication
- CSRF on sensitive endpoints

The following are **out of scope**:

- Attacks that require physical access to the device (assumed hostile)
- Denial of service against the ESP8266 itself (it is a microcontroller)
- Issues in upstream libraries (ESP8266 Arduino framework, LittleFS) — report those upstream
- The fact that the tool can be used for offensive purposes — that is by design for authorized use
