#!/bin/sh
#
# ==========================
# Construye el .pkg de macOS
# ==========================
#
#   sh packaging/macos/build-pkg.sh <ruta-al-binario-tz> [carpeta-de-salida]
#
# Produce un instalador grafico con
# asistente, igual que el de
# python.org: el usuario hace doble
# clic, pulsa Continuar y termina con
# 'tz' en /usr/local/bin, que ya esta
# en el PATH de macOS por defecto.
#
# No necesita nada instalado:
# pkgbuild y productbuild vienen con
# las herramientas de linea de
# ordenes de Xcode.
#
# ==========================
# FIRMA (opcional)
# ==========================
#
# Sin firmar, el instalador funciona
# pero macOS avisa la primera vez y
# hay que abrirlo con clic derecho >
# Abrir. Para evitarlo hace falta una
# cuenta de Apple Developer.
#
# Si existe la variable
# MACOS_INSTALLER_IDENTITY, el
# paquete se firma con ella:
#
#   MACOS_INSTALLER_IDENTITY="Developer ID Installer: Nombre (TEAMID)" \
#     sh packaging/macos/build-pkg.sh build/tz

set -eu

BINARIO="${1:?Uso: build-pkg.sh <ruta-al-binario-tz> [salida]}"
SALIDA="${2:-dist}"

RAIZ="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
AQUI="$RAIZ/packaging/macos"

[ -f "$BINARIO" ] || { echo "Error: no existe $BINARIO" >&2; exit 1; }

# La version sale de src/version.h,
# igual que en el resto del proyecto.

VERSION="$(sed -n 's/.*TZLANG_VERSION "\([0-9.]*\)".*/\1/p' "$RAIZ/src/version.h")"
[ -n "$VERSION" ] || { echo "Error: no se pudo leer la version" >&2; exit 1; }

IDENTIFICADOR="com.tzerk.tzlang"
PKG_FINAL="$SALIDA/TzLang-v$VERSION-macos.pkg"

echo "TzLang $VERSION -> $PKG_FINAL"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT INT TERM

# ==========================
# 1. PAYLOAD
# ==========================
#
# El arbol de archivos tal cual
# quedara en el disco del usuario.

mkdir -p "$TMP/root/usr/local/bin"
cp "$BINARIO" "$TMP/root/usr/local/bin/tz"
chmod 755 "$TMP/root/usr/local/bin/tz"

mkdir -p "$TMP/root/usr/local/share/doc/tzlang"
cp "$RAIZ/LICENSE" "$RAIZ/README.md" "$TMP/root/usr/local/share/doc/tzlang/"

# ==========================
# TzLang.app
# ==========================
#
# En macOS el icono de un tipo de
# archivo SOLO se puede declarar desde
# el Info.plist de una aplicacion. Esta
# es minima: recibe los .tz y los
# ejecuta en el Terminal, con lo que
# ademas da doble clic para ejecutar.
#
# Va a /Applications porque
# LaunchServices registra solo lo que
# encuentra ahi.

sh "$RAIZ/packaging/macos/app/construir-app.sh" "$TMP/apps" >/dev/null

mkdir -p "$TMP/root/Applications"
cp -R "$TMP/apps/TzLang.app" "$TMP/root/Applications/"

# ==========================
# 2. COMPONENTE
# ==========================

pkgbuild \
    --root "$TMP/root" \
    --identifier "$IDENTIFICADOR" \
    --version "$VERSION" \
    --install-location "/" \
    "$TMP/tzlang-component.pkg"

# ==========================
# 3. INSTALADOR CON ASISTENTE
# ==========================
#
# distribution.xml es lo que convierte
# el componente en un asistente con
# pantallas de bienvenida, licencia y
# despedida.

mkdir -p "$SALIDA"

# La version se inyecta en la
# plantilla para no repetirla.
sed "s/@VERSION@/$VERSION/g" \
    "$AQUI/distribution.xml" > "$TMP/distribution.xml"

# La licencia se copia a resources/
# porque productbuild busca ahi todo
# lo que menciona el XML.
cp "$AQUI/resources/welcome.html" \
   "$AQUI/resources/conclusion.html" \
   "$TMP/"
cp "$RAIZ/LICENSE" "$TMP/LICENSE.txt"

if [ -n "${MACOS_INSTALLER_IDENTITY:-}" ]; then
    echo "Firmando con: $MACOS_INSTALLER_IDENTITY"
    productbuild \
        --distribution "$TMP/distribution.xml" \
        --resources "$TMP" \
        --package-path "$TMP" \
        --sign "$MACOS_INSTALLER_IDENTITY" \
        "$PKG_FINAL"
else
    echo "Sin firmar (define MACOS_INSTALLER_IDENTITY para firmarlo)."
    productbuild \
        --distribution "$TMP/distribution.xml" \
        --resources "$TMP" \
        --package-path "$TMP" \
        "$PKG_FINAL"
fi

echo "Listo: $PKG_FINAL"
