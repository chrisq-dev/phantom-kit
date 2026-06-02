#!/usr/bin/env bash
# =============================================================================
# PhantomKit — Release v1.0.0
# Ejecutar desde la raiz del repositorio: bash scripts/release.sh
# Requiere: gh CLI autenticado (gh auth login) y git configurado
# =============================================================================

set -e

REPO="chrisq-dev/phantom-kit"
TAG="v1.0.0"
TITLE="PhantomKit v1.0.0 — Initial Release"

echo "PhantomKit — Release Automation"
echo "================================"

# -- 1. Commit de archivos nuevos ---------------------------------------------
echo "[1/5] Preparando commit..."
git add README.md README.es.md CHANGELOG.md
git commit -m "docs: improve README, add Spanish version and CHANGELOG for v1.0.0

- README.md rewritten in English with language switcher (EN/ES)
- README.es.md added as Spanish version
- Both READMEs include module table, audit scenarios, roadmap linked to issues
- CHANGELOG.md added following Keep a Changelog convention"

echo "Commit creado."

# -- 2. Push ------------------------------------------------------------------
echo "[2/5] Push a origin/main..."
git push origin main
echo "Push completado."

# -- 3. Crear tag v1.0.0 ------------------------------------------------------
echo "[3/5] Creando tag $TAG..."
git tag -a "$TAG" -m "PhantomKit v1.0.0 — Initial release

Full Wi-Fi auditing suite for ESP8266:
- Evil Portal with 8 cloned templates
- Deauth Attack, Beacon Flood, Probe Sniffer, Evil Twin
- Web dashboard with real-time capture and live logs
- Automatic channel hopping (channels 1-13)
- Auto-Portal with environment analysis"
git push origin "$TAG"
echo "Tag $TAG publicado."

# -- 4. Crear GitHub Release --------------------------------------------------
echo "[4/5] Creando GitHub Release..."
gh release create "$TAG" \
  --repo "$REPO" \
  --title "$TITLE" \
  --notes "## ESP8266 PhantomKit v1.0.0

First stable release of PhantomKit — self-contained Wi-Fi auditing platform for ESP8266.

### Included in this release

- Evil Portal with 8 cloned templates (Facebook, Instagram, Microsoft, X, Google, WiFi Login, Netflix, WhatsApp)
- Web dashboard at \`http://192.168.4.1/dashboard\`
- Real-time credential capture
- Deauth Attack — IEEE 802.11 deauth packets
- Beacon Flood — 50+ fake SSIDs
- Probe Sniffer — passive probe request capture
- Evil Twin — clone existing networks
- Automatic channel hopping (channels 1-13)
- Auto-Portal — environment analysis and template recommendations

### Quick start

\`\`\`bash
git clone https://github.com/chrisq-dev/phantom-kit.git
cd phantom-kit
pio run --target uploadfs && pio run --target upload
\`\`\`

Connect to the **PhantomKit** network (password: \`phantom123\`) and open **http://192.168.4.1/dashboard**

---

For authorized security audits and educational use only. See [docs/legal.md](docs/legal.md) for the full disclaimer."

echo "Release $TAG publicado."

# -- 5. Topics del repo -------------------------------------------------------
echo "[5/5] Configurando topics del repositorio..."
gh repo edit "$REPO" \
  --add-topic esp8266 \
  --add-topic security \
  --add-topic wifi \
  --add-topic pentesting \
  --add-topic iot \
  --add-topic evil-portal \
  --add-topic captive-portal \
  --add-topic arduino \
  --add-topic cybersecurity \
  --add-topic platformio
echo "Topics configurados."

echo ""
echo "Release v1.0.0 completado."
echo "Repo: https://github.com/$REPO"
