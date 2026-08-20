#!/bin/sh
#
# ==========================
# Construye el .deb (Debian, Ubuntu, Mint...)
# ==========================
#
#   sh packaging/linux/build-deb.sh <ruta-al-binario-tz> <arquitectura> [salida]
#
#   arquitectura: amd64 o arm64
#
# El usuario lo instala con doble clic
# desde el explorador de archivos, o
# con:
#
#   sudo apt install ./tzlang_0.1.0_amd64.deb
#
# y queda 'tz' en /usr/bin, dentro del
# PATH, y registrado en el gestor de
# paquetes: 'apt remove tzlang' lo
# quita limpiamente.
#
# Solo necesita dpkg-deb, que viene en
# cualquier Debian o Ubuntu.

set -eu

BINARIO="${1:?Uso: build-deb.sh <binario> <amd64|arm64> [salida]}"
ARCO="${2:?Falta la arquitectura: amd64 o arm64}"
SALIDA="${3:-dist}"

RAIZ="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"

[ -f "$BINARIO" ] || { echo "Error: no existe $BINARIO" >&2; exit 1; }

case "$ARCO" in
    amd64|arm64) ;;
    *) echo "Error: arquitectura no valida: $ARCO (usa amd64 o arm64)" >&2; exit 1 ;;
esac

VERSION="$(sed -n 's/.*TZLANG_VERSION "\([0-9.]*\)".*/\1/p' "$RAIZ/src/version.h")"
[ -n "$VERSION" ] || { echo "Error: no se pudo leer la version" >&2; exit 1; }

DEB="$SALIDA/tzlang_${VERSION}_${ARCO}.deb"

echo "TzLang $VERSION ($ARCO) -> $DEB"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT INT TERM

ROOT="$TMP/root"

# ==========================
# ARBOL DE ARCHIVOS
# ==========================

mkdir -p "$ROOT/usr/bin"
cp "$BINARIO" "$ROOT/usr/bin/tz"
chmod 755 "$ROOT/usr/bin/tz"

mkdir -p "$ROOT/usr/share/doc/tzlang"
cp "$RAIZ/README.md" "$ROOT/usr/share/doc/tzlang/"

# ==========================
# ICONO Y TIPO DE ARCHIVO
# ==========================
#
# Para que el explorador de archivos
# reconozca los .tz y les ponga el
# icono hacen falta dos cosas: declarar
# el tipo MIME y dejar el icono con el
# nombre que ese tipo espera.

mkdir -p "$ROOT/usr/share/mime/packages"
cp "$RAIZ/packaging/linux/tzlang-mime.xml" \
   "$ROOT/usr/share/mime/packages/tzlang.xml"

for t in 16 24 32 48 64 128 256; do
    destino="$ROOT/usr/share/icons/hicolor/${t}x${t}"
    mkdir -p "$destino/mimetypes" "$destino/apps"
    cp "$RAIZ/packaging/icono/tzlang-$t.png" \
       "$destino/mimetypes/text-x-tzlang.png"
    cp "$RAIZ/packaging/icono/tzlang-$t.png" \
       "$destino/apps/tzlang.png"
done

# Debian exige un archivo 'copyright'
# con este nombre exacto.
cp "$RAIZ/LICENSE" "$ROOT/usr/share/doc/tzlang/copyright"

chmod -R go-w "$ROOT"

# ==========================
# REFRESCAR LAS CACHES
# ==========================
#
# Sin esto el sistema no se entera de
# que hay un tipo y un icono nuevos
# hasta el siguiente reinicio de sesion.

mkdir -p "$ROOT/DEBIAN"

cat > "$ROOT/DEBIAN/postinst" <<'POSTINST'
#!/bin/sh
set -e
if command -v update-mime-database >/dev/null 2>&1; then
    update-mime-database /usr/share/mime >/dev/null 2>&1 || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t /usr/share/icons/hicolor >/dev/null 2>&1 || true
fi
exit 0
POSTINST

cat > "$ROOT/DEBIAN/postrm" <<'POSTRM'
#!/bin/sh
set -e
if command -v update-mime-database >/dev/null 2>&1; then
    update-mime-database /usr/share/mime >/dev/null 2>&1 || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t /usr/share/icons/hicolor >/dev/null 2>&1 || true
fi
exit 0
POSTRM

chmod 755 "$ROOT/DEBIAN/postinst" "$ROOT/DEBIAN/postrm"

# ==========================
# METADATOS
# ==========================
#
# El binario se enlaza estaticamente,
# asi que no lleva Depends: no hay
# ninguna biblioteca que exigir.

mkdir -p "$ROOT/DEBIAN"

# Installed-Size va en kilobytes y lo
# muestra el gestor de paquetes.
TAMANO="$(du -ks "$ROOT/usr" | cut -f1)"

cat > "$ROOT/DEBIAN/control" <<CONTROL
Package: tzlang
Version: $VERSION
Section: devel
Priority: optional
Architecture: $ARCO
Maintainer: TzLanguaje <https://github.com/TzLanguaje>
Homepage: https://github.com/TzLanguaje/TzLanguaje
Installed-Size: $TAMANO
Description: Lenguaje de programacion educativo con sintaxis en espanol
 TzLang es un lenguaje interpretado pensado para aprender a programar
 sin la barrera del ingles: las palabras clave y las comparaciones se
 escriben como se dicen en voz alta.
 .
 Esta escrito desde cero en C11, sin dependencias externas ni
 generadores de parsers. Los programas se guardan con extension .tz y
 se ejecutan con la orden 'tz'.
CONTROL

mkdir -p "$SALIDA"

# --root-owner-group evita que el
# paquete herede el usuario que lo
# construyo: todo queda de root.
dpkg-deb --build --root-owner-group "$ROOT" "$DEB"

echo "Listo: $DEB"
dpkg-deb --info "$DEB" | sed 's/^/  /'
