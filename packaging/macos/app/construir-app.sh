#!/bin/sh
#
# ==========================
# Construye TzLang.app
# ==========================
#
#   sh packaging/macos/app/construir-app.sh [carpeta-de-salida]
#
# En macOS, poner icono a un tipo de
# archivo SOLO se puede declarandolo
# desde el Info.plist de una
# aplicacion. Un binario de linea de
# ordenes no tiene donde declararlo.
#
# Esta aplicacion es minima: recibe
# archivos .tz y los ejecuta en el
# Terminal. Con eso se gana el icono y,
# de paso, el doble clic para ejecutar.

set -eu

AQUI="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
RAIZ="$(CDPATH= cd -- "$AQUI/../../.." && pwd)"
SALIDA="${1:-$RAIZ/dist/macos}"

VERSION="$(sed -n 's/.*TZLANG_VERSION "\([0-9.]*\)".*/\1/p' "$RAIZ/src/version.h")"

APP="$SALIDA/TzLang.app"

echo "TzLang $VERSION -> $APP"

mkdir -p "$SALIDA"
rm -rf "$APP"

osacompile -o "$APP" "$AQUI/TzLang.applescript"

# ==========================
# ICONO
# ==========================

cp "$RAIZ/packaging/icono/tzlang.icns" "$APP/Contents/Resources/tzlang.icns"

# ==========================
# INFO.PLIST
# ==========================
#
# UTExportedTypeDeclarations declara
# que existe un tipo llamado
# com.tzlanguaje.tz, que son los .tz y
# que su icono es tzlang.icns.
#
# CFBundleDocumentTypes dice que esta
# aplicacion sabe abrirlos, que es lo
# que hace funcionar el doble clic.

PLIST="$APP/Contents/Info.plist"

/usr/libexec/PlistBuddy -c "Set :CFBundleName TzLang" "$PLIST" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Add :CFBundleName string TzLang" "$PLIST"

/usr/libexec/PlistBuddy -c "Add :CFBundleIdentifier string com.tzlanguaje.TzLang" "$PLIST" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Set :CFBundleIdentifier com.tzlanguaje.TzLang" "$PLIST"

/usr/libexec/PlistBuddy -c "Set :CFBundleIconFile tzlang" "$PLIST" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Add :CFBundleIconFile string tzlang" "$PLIST"

/usr/libexec/PlistBuddy -c "Add :CFBundleShortVersionString string $VERSION" "$PLIST" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString $VERSION" "$PLIST"

# osacompile ya deja algunas de estas
# claves puestas. Se borran primero
# para no chocar con las nuestras.

/usr/libexec/PlistBuddy -c "Delete :CFBundleDocumentTypes" "$PLIST" 2>/dev/null || true
/usr/libexec/PlistBuddy -c "Delete :UTExportedTypeDeclarations" "$PLIST" 2>/dev/null || true

# --- el tipo .tz ---

/usr/libexec/PlistBuddy \
    -c "Add :UTExportedTypeDeclarations array" \
    -c "Add :UTExportedTypeDeclarations:0 dict" \
    -c "Add :UTExportedTypeDeclarations:0:UTTypeIdentifier string com.tzlanguaje.tz" \
    -c "Add :UTExportedTypeDeclarations:0:UTTypeDescription string Programa de TzLang" \
    -c "Add :UTExportedTypeDeclarations:0:UTTypeIconFile string tzlang" \
    -c "Add :UTExportedTypeDeclarations:0:UTTypeConformsTo array" \
    -c "Add :UTExportedTypeDeclarations:0:UTTypeConformsTo:0 string public.plain-text" \
    -c "Add :UTExportedTypeDeclarations:0:UTTypeConformsTo:1 string public.source-code" \
    -c "Add :UTExportedTypeDeclarations:0:UTTypeTagSpecification dict" \
    -c "Add :UTExportedTypeDeclarations:0:UTTypeTagSpecification:public.filename-extension array" \
    -c "Add :UTExportedTypeDeclarations:0:UTTypeTagSpecification:public.filename-extension:0 string tz" \
    "$PLIST"

# --- que esta app los abre ---

/usr/libexec/PlistBuddy \
    -c "Add :CFBundleDocumentTypes array" \
    -c "Add :CFBundleDocumentTypes:0 dict" \
    -c "Add :CFBundleDocumentTypes:0:CFBundleTypeName string Programa de TzLang" \
    -c "Add :CFBundleDocumentTypes:0:CFBundleTypeRole string Editor" \
    -c "Add :CFBundleDocumentTypes:0:CFBundleTypeIconFile string tzlang" \
    -c "Add :CFBundleDocumentTypes:0:LSHandlerRank string Owner" \
    -c "Add :CFBundleDocumentTypes:0:LSItemContentTypes array" \
    -c "Add :CFBundleDocumentTypes:0:LSItemContentTypes:0 string com.tzlanguaje.tz" \
    "$PLIST"

plutil -lint "$PLIST" >/dev/null

# Firmar de forma local (ad-hoc). Sin
# esto macOS desconfia mas de la app.
codesign --force --deep --sign - "$APP" 2>/dev/null || true

echo "Listo: $APP"
