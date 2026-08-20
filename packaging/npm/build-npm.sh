#!/bin/sh
#
# ==========================
# Prepara los paquetes de npm
# ==========================
#
#   sh packaging/npm/build-npm.sh <carpeta-con-los-artefactos> [salida]
#
# La carpeta de entrada es la que
# produce el workflow de release, con
# los .tar.gz y el .zip dentro.
#
# Genera CINCO paquetes:
#
#   tzlang              el lanzador, sin binario
#   tzlang-darwin       binario universal de macOS
#   tzlang-linux-x64
#   tzlang-linux-arm64
#   tzlang-win32-x64
#
# Los cuatro de plataforma declaran
# 'os' y 'cpu', asi que npm instala
# solo el que corresponde. El
# principal los lista como
# optionalDependencies.
#
# Este script NO publica nada.

set -eu

ARTEFACTOS="${1:?Uso: build-npm.sh <carpeta-con-artefactos> [salida]}"
SALIDA="${2:-dist/npm}"

RAIZ="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
AQUI="$RAIZ/packaging/npm"

VERSION="$(sed -n 's/.*TZLANG_VERSION "\([0-9.]*\)".*/\1/p' "$RAIZ/src/version.h")"
[ -n "$VERSION" ] || { echo "Error: no se pudo leer la version" >&2; exit 1; }

REPO_URL="https://github.com/TzLanguaje/TzLanguaje"
DESC="TzLang: lenguaje de programación educativo con sintaxis en español"

echo "TzLang $VERSION -> paquetes npm en $SALIDA"

rm -rf "$SALIDA"
mkdir -p "$SALIDA"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT INT TERM

# ==========================
# Extraer los binarios
# ==========================

extraer_tar() {
    # $1 destino  $2 archivo tar.gz
    nombre="tzlang-v$VERSION-$2"
    [ -f "$ARTEFACTOS/$nombre.tar.gz" ] || {
        echo "Error: falta $ARTEFACTOS/$nombre.tar.gz" >&2
        exit 1
    }
    mkdir -p "$TMP/$2"
    tar -xzf "$ARTEFACTOS/$nombre.tar.gz" -C "$TMP/$2"
    cp "$TMP/$2/$nombre/tz" "$1/bin/tz"
    chmod 755 "$1/bin/tz"
}

# ==========================
# Un paquete de plataforma
# ==========================
#
# $1 nombre  $2 os  $3 cpu (JSON)

paquete_plataforma() {
    dir="$SALIDA/$1"
    mkdir -p "$dir/bin"

    cat > "$dir/package.json" <<JSON
{
  "name": "$1",
  "version": "$VERSION",
  "description": "$DESC — binario para $2",
  "license": "MIT",
  "homepage": "$REPO_URL",
  "repository": {
    "type": "git",
    "url": "git+$REPO_URL.git"
  },
  "os": ["$2"],
  "cpu": $3,
  "files": ["bin"],
  "preferUnplugged": true
}
JSON

    cp "$RAIZ/LICENSE" "$dir/LICENSE"
    echo "   $1"
}

paquete_plataforma "tzlang-darwin"     "darwin" '["x64", "arm64"]'
extraer_tar "$SALIDA/tzlang-darwin" "macos-universal"

paquete_plataforma "tzlang-linux-x64"  "linux"  '["x64"]'
extraer_tar "$SALIDA/tzlang-linux-x64" "linux-x86_64"

paquete_plataforma "tzlang-linux-arm64" "linux" '["arm64"]'
extraer_tar "$SALIDA/tzlang-linux-arm64" "linux-aarch64"

paquete_plataforma "tzlang-win32-x64"  "win32"  '["x64"]'

zip_win="$ARTEFACTOS/tzlang-v$VERSION-windows-x86_64.zip"
[ -f "$zip_win" ] || { echo "Error: falta $zip_win" >&2; exit 1; }
mkdir -p "$TMP/win"
unzip -q "$zip_win" -d "$TMP/win"
cp "$(find "$TMP/win" -name 'tz.exe' | head -n 1)" "$SALIDA/tzlang-win32-x64/bin/tz.exe"

# ==========================
# El paquete principal
# ==========================

dir="$SALIDA/tzlang"
mkdir -p "$dir/bin"

cp "$AQUI/plantilla/bin/tz.js" "$dir/bin/tz.js"
chmod 755 "$dir/bin/tz.js"
cp "$RAIZ/LICENSE" "$dir/LICENSE"
cp "$RAIZ/README.md" "$dir/README.md"

cat > "$dir/package.json" <<JSON
{
  "name": "tzlang",
  "version": "$VERSION",
  "description": "$DESC",
  "license": "MIT",
  "homepage": "$REPO_URL",
  "repository": {
    "type": "git",
    "url": "git+$REPO_URL.git"
  },
  "bugs": {
    "url": "$REPO_URL/issues"
  },
  "keywords": [
    "tzlang",
    "lenguaje",
    "espanol",
    "interprete",
    "educativo",
    "programacion"
  ],
  "bin": {
    "tz": "bin/tz.js"
  },
  "files": ["bin", "LICENSE", "README.md"],
  "engines": {
    "node": ">=14"
  },
  "optionalDependencies": {
    "tzlang-darwin": "$VERSION",
    "tzlang-linux-x64": "$VERSION",
    "tzlang-linux-arm64": "$VERSION",
    "tzlang-win32-x64": "$VERSION"
  }
}
JSON

echo "   tzlang"
echo "Listo."
