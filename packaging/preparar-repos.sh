#!/bin/sh
#
# ==========================
# Prepara los repositorios de
# Homebrew y Scoop
# ==========================
#
#   sh packaging/preparar-repos.sh [carpeta-de-salida]
#
# Deja dos arboles listos para subir a
# GitHub, con los sha256 ya puestos:
#
#   <salida>/homebrew-tzlang/
#       Formula/tzlang.rb
#       README.md
#
#   <salida>/scoop-tzlang/
#       bucket/tzlang.json
#       README.md
#
# Los sha256 se descargan de la
# release publicada. Si no hay red, el
# script termina igual pero deja los
# huecos marcados y te dice como
# rellenarlos a mano.
#
# Requiere que la release de la
# version actual YA este publicada.

set -eu

RAIZ="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
SALIDA="${1:-$RAIZ/dist/repos}"

VERSION="$(sed -n 's/.*TZLANG_VERSION "\([0-9.]*\)".*/\1/p' "$RAIZ/src/version.h")"
[ -n "$VERSION" ] || { echo "Error: no se pudo leer la version" >&2; exit 1; }

REPO="TzLanguaje/TzLanguaje"
BASE="https://github.com/$REPO"

echo "TzLang $VERSION"
echo

# ==========================
# HERRAMIENTAS
# ==========================

if command -v curl >/dev/null 2>&1; then
    leer() { curl -fsSL "$1" 2>/dev/null; }
    bajar() { curl -fsSL "$1" -o "$2" 2>/dev/null; }
elif command -v wget >/dev/null 2>&1; then
    leer() { wget -qO - "$1" 2>/dev/null; }
    bajar() { wget -qO "$2" "$1" 2>/dev/null; }
else
    echo "Error: hace falta curl o wget" >&2
    exit 1
fi

sha256() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$1" | cut -d' ' -f1
    else
        shasum -a 256 "$1" | cut -d' ' -f1
    fi
}

FALTAN=0

# ==========================
# SHA256 DEL CODIGO FUENTE
# ==========================
#
# Lo necesita Homebrew, que compila
# desde el codigo en vez de descargar
# el binario.

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT INT TERM

echo "Descargando el codigo de v$VERSION..."

if bajar "$BASE/archive/refs/tags/v$VERSION.tar.gz" "$TMP/fuente.tar.gz"; then
    SHA_FUENTE="$(sha256 "$TMP/fuente.tar.gz")"
    echo "   sha256: $SHA_FUENTE"
else
    SHA_FUENTE="RELLENAR_VER_README"
    FALTAN=1
    echo "   no se pudo descargar (sin red, o la release no existe todavia)"
fi

# ==========================
# SHA256 DEL ZIP DE WINDOWS
# ==========================
#
# Lo necesita Scoop. Sale del
# SHA256SUMS.txt que publica la
# release.

ZIP="tzlang-v$VERSION-windows-x86_64.zip"

echo "Buscando el checksum de $ZIP..."

if bajar "$BASE/releases/download/v$VERSION/SHA256SUMS.txt" "$TMP/sumas.txt"; then
    patron="$(printf '%s' "$ZIP" | sed 's/[.]/[.]/g')"
    SHA_ZIP="$(sed -n "s/^\([0-9a-f]\{64\}\)[[:space:]][[:space:]]*[*]\{0,1\}$patron\$/\1/p" "$TMP/sumas.txt" | head -n 1)"
    if [ -n "$SHA_ZIP" ]; then
        echo "   sha256: $SHA_ZIP"
    else
        SHA_ZIP="RELLENAR_VER_README"
        FALTAN=1
        echo "   el zip no aparece en SHA256SUMS.txt"
    fi
else
    SHA_ZIP="RELLENAR_VER_README"
    FALTAN=1
    echo "   no se pudo descargar SHA256SUMS.txt"
fi

echo

# ==========================
# REPOSITORIO DE HOMEBREW
# ==========================
#
# El nombre NO es libre: Homebrew
# exige el prefijo 'homebrew-', y lo
# da por supuesto al instalar.

H="$SALIDA/homebrew-tzlang"
mkdir -p "$H/Formula"

sed -e "s|PON_AQUI_EL_SHA256_DEL_TARBALL|$SHA_FUENTE|" \
    "$RAIZ/packaging/homebrew/tzlang.rb" > "$H/Formula/tzlang.rb"

cat > "$H/README.md" <<HREADME
# homebrew-tzlang

Tap de Homebrew para
[TzLang]($BASE), un lenguaje de
programación educativo con sintaxis
en español.

## Instalación

\`\`\`bash
brew install tzlanguaje/tzlang/tzlang
\`\`\`

Compila desde el código, así que sirve
para macOS (Intel y Apple Silicon) y
para Linux.

## Comprobar

\`\`\`bash
tz --version
\`\`\`

## Actualizar a una versión nueva

En \`Formula/tzlang.rb\`, cambiar la
\`url\` a la etiqueta nueva y poner su
\`sha256\`:

\`\`\`bash
curl -fsSL $BASE/archive/refs/tags/vX.Y.Z.tar.gz | shasum -a 256
\`\`\`

## Licencia

MIT, igual que TzLang.
HREADME

echo "Listo: $H"

# ==========================
# REPOSITORIO DE SCOOP
# ==========================

S="$SALIDA/scoop-tzlang"
mkdir -p "$S/bucket"

sed -e "s|PON_AQUI_EL_SHA256_DEL_ZIP|$SHA_ZIP|" \
    "$RAIZ/packaging/scoop/tzlang.json" > "$S/bucket/tzlang.json"

cat > "$S/README.md" <<SREADME
# scoop-tzlang

Bucket de Scoop para
[TzLang]($BASE), un lenguaje de
programación educativo con sintaxis
en español.

## Instalación

\`\`\`powershell
scoop bucket add tzlang https://github.com/TzLanguaje/scoop-tzlang
scoop install tzlang
\`\`\`

## Comprobar

\`\`\`powershell
tz --version
\`\`\`

## Actualizar

El manifiesto lleva \`checkver\` y
\`autoupdate\`, así que \`scoop update\`
encuentra las versiones nuevas sin
tocarlo a mano.

## Licencia

MIT, igual que TzLang.
SREADME

echo "Listo: $S"
echo

# ==========================
# QUE HACER AHORA
# ==========================

cat <<FIN
Para subirlos, uno por uno:

  cd $H
  git init -b main
  git add -A
  git commit -m "Formula de TzLang $VERSION"
  git remote add origin https://github.com/TzLanguaje/homebrew-tzlang.git
  git push -u origin main

  cd $S
  git init -b main
  git add -A
  git commit -m "Manifiesto de TzLang $VERSION"
  git remote add origin https://github.com/TzLanguaje/scoop-tzlang.git
  git push -u origin main

Los dos repositorios hay que crearlos
antes, vacios, en github.com/TzLanguaje
FIN

if [ "$FALTAN" = "1" ]; then
    cat <<AVISO

AVISO: falta algun sha256, marcado como
RELLENAR_VER_README. Se rellenan asi:

  Homebrew (codigo fuente):
    curl -fsSL $BASE/archive/refs/tags/v$VERSION.tar.gz | shasum -a 256

  Scoop (zip de Windows):
    la linea de $ZIP
    en $BASE/releases/download/v$VERSION/SHA256SUMS.txt

O vuelve a ejecutar este script cuando
la release este publicada y haya red.
AVISO
fi
