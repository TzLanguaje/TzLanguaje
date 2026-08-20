#!/bin/sh
#
# ==========================
# Construye el .rpm (Fedora, RHEL, openSUSE...)
# ==========================
#
#   sh packaging/linux/build-rpm.sh <ruta-al-binario-tz> <arquitectura> [salida]
#
#   arquitectura: x86_64 o aarch64
#
# El usuario lo instala con doble clic
# o con:
#
#   sudo dnf install ./tzlang-0.1.0-1.x86_64.rpm
#
# Necesita rpmbuild. En Debian/Ubuntu
# viene en el paquete 'rpm':
#
#   sudo apt install rpm

set -eu

BINARIO="${1:?Uso: build-rpm.sh <binario> <x86_64|aarch64> [salida]}"
ARCO="${2:?Falta la arquitectura: x86_64 o aarch64}"
SALIDA="${3:-dist}"

RAIZ="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"

[ -f "$BINARIO" ] || { echo "Error: no existe $BINARIO" >&2; exit 1; }

case "$ARCO" in
    x86_64|aarch64) ;;
    *) echo "Error: arquitectura no valida: $ARCO" >&2; exit 1 ;;
esac

VERSION="$(sed -n 's/.*TZLANG_VERSION "\([0-9.]*\)".*/\1/p' "$RAIZ/src/version.h")"
[ -n "$VERSION" ] || { echo "Error: no se pudo leer la version" >&2; exit 1; }

echo "TzLang $VERSION ($ARCO) -> rpm"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT INT TERM

mkdir -p "$TMP/BUILD" "$TMP/RPMS" "$TMP/SOURCES" "$TMP/SPECS"

cp "$BINARIO"        "$TMP/SOURCES/tz"
cp "$RAIZ/LICENSE"   "$TMP/SOURCES/LICENSE"
cp "$RAIZ/README.md" "$TMP/SOURCES/README.md"
cp "$RAIZ/packaging/linux/tzlang-mime.xml" "$TMP/SOURCES/tzlang.xml"

for t in 16 24 32 48 64 128 256; do
    cp "$RAIZ/packaging/icono/tzlang-$t.png" "$TMP/SOURCES/icono-$t.png"
done

# ==========================
# SPEC
# ==========================
#
# El binario ya viene compilado, asi
# que no hay %build: solo se instala.
#
# AutoReqProv: no -> el binario es
# estatico y no debe declarar
# dependencias de bibliotecas.

cat > "$TMP/SPECS/tzlang.spec" <<SPEC
Name:           tzlang
Version:        $VERSION
Release:        1
Summary:        Lenguaje de programacion educativo con sintaxis en espanol

License:        MIT
URL:            https://github.com/TzLanguaje/TzLanguaje

Source0:        tz
Source1:        LICENSE
Source2:        README.md
Source3:        tzlang.xml

BuildArch:      $ARCO
AutoReqProv:    no

%description
TzLang es un lenguaje interpretado pensado para aprender a programar
sin la barrera del ingles: las palabras clave y las comparaciones se
escriben como se dicen en voz alta.

Esta escrito desde cero en C11, sin dependencias externas ni
generadores de parsers. Los programas se guardan con extension .tz y
se ejecutan con la orden 'tz'.

%install
install -D -m 0755 %{SOURCE0} %{buildroot}%{_bindir}/tz
install -D -m 0644 %{SOURCE1} %{buildroot}%{_datadir}/doc/tzlang/LICENSE
install -D -m 0644 %{SOURCE2} %{buildroot}%{_datadir}/doc/tzlang/README.md

# Tipo de archivo e iconos, para que el
# explorador reconozca los .tz
install -D -m 0644 %{SOURCE3} %{buildroot}%{_datadir}/mime/packages/tzlang.xml

for t in 16 24 32 48 64 128 256; do
    install -D -m 0644 %{_sourcedir}/icono-$t.png \
        %{buildroot}%{_datadir}/icons/hicolor/${t}x${t}/mimetypes/text-x-tzlang.png
    install -D -m 0644 %{_sourcedir}/icono-$t.png \
        %{buildroot}%{_datadir}/icons/hicolor/${t}x${t}/apps/tzlang.png
done

%post
update-mime-database %{_datadir}/mime >/dev/null 2>&1 || :
gtk-update-icon-cache -f -t %{_datadir}/icons/hicolor >/dev/null 2>&1 || :

%postun
update-mime-database %{_datadir}/mime >/dev/null 2>&1 || :
gtk-update-icon-cache -f -t %{_datadir}/icons/hicolor >/dev/null 2>&1 || :

%files
%{_bindir}/tz
%{_datadir}/doc/tzlang/LICENSE
%{_datadir}/doc/tzlang/README.md
%{_datadir}/mime/packages/tzlang.xml
%{_datadir}/icons/hicolor/*/mimetypes/text-x-tzlang.png
%{_datadir}/icons/hicolor/*/apps/tzlang.png

%changelog
SPEC

# ==========================
# CONSTRUIR
# ==========================
#
# --define _topdir mantiene todo
# dentro del temporal, sin tocar
# ~/rpmbuild del usuario.

rpmbuild \
    --define "_topdir $TMP" \
    --define "_binary_payload w2.xzdio" \
    --target "$ARCO" \
    -bb "$TMP/SPECS/tzlang.spec" > "$TMP/rpmbuild.log" 2>&1 ||
    { echo "Error: fallo rpmbuild" >&2; cat "$TMP/rpmbuild.log" >&2; exit 1; }

mkdir -p "$SALIDA"

RPM="$(find "$TMP/RPMS" -name '*.rpm' -type f | head -n 1)"
[ -n "$RPM" ] || { echo "Error: rpmbuild no genero ningun .rpm" >&2; exit 1; }

cp "$RPM" "$SALIDA/"

echo "Listo: $SALIDA/$(basename "$RPM")"
rpm -qip "$SALIDA/$(basename "$RPM")" 2>/dev/null | sed 's/^/  /' || true
