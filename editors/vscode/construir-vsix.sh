#!/bin/sh
#
# ==========================
# Construye la extension de VS Code
# ==========================
#
#   sh editors/vscode/construir-vsix.sh [carpeta-de-salida]
#
# Deja un .vsix en dist/ listo para
# instalar con:
#
#   code --install-extension dist/tzlang-X.Y.Z.vsix
#
# Necesita node, solo para empaquetar.
# La extension en si no lleva codigo:
# es un manifiesto, un icono y la
# configuracion del lenguaje.

set -eu

AQUI="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
RAIZ="$(CDPATH= cd -- "$AQUI/../.." && pwd)"
SALIDA="${1:-$RAIZ/dist}"

command -v node >/dev/null 2>&1 || {
    echo "Error: hace falta node para empaquetar el .vsix." >&2
    echo "Se instala desde https://nodejs.org" >&2
    exit 1
}

VERSION="$(sed -n 's/.*"version"[[:space:]]*:[[:space:]]*"\([0-9.]*\)".*/\1/p' \
           "$AQUI/package.json" | head -1)"

[ -n "$VERSION" ] || { echo "Error: no se pudo leer la version" >&2; exit 1; }

# La version del manifiesto y la del
# lenguaje van juntas: subir-version.sh
# las mueve a la vez. Si se separan es
# que alguien edito una a mano.
DEL_LENGUAJE="$(sed -n 's/.*TZLANG_VERSION "\([0-9.]*\)".*/\1/p' "$RAIZ/src/version.h")"

if [ "$VERSION" != "$DEL_LENGUAJE" ]; then
    echo "Error: la extension dice $VERSION y TzLang dice $DEL_LENGUAJE." >&2
    echo "Usa scripts/subir-version.sh para moverlas a la vez." >&2
    exit 1
fi

echo "TzLang $VERSION (extension de VS Code)"
echo

# ==========================
# LA LICENCIA
# ==========================
#
# vsce avisa si el paquete no lleva
# licencia dentro. Se copia al empezar
# y se borra al terminar: en el
# repositorio la licencia vive en un
# solo sitio, la raiz.

cp "$RAIZ/LICENSE" "$AQUI/LICENSE.txt"

limpiar() { rm -f "$AQUI/LICENSE.txt"; }
trap limpiar EXIT INT TERM

mkdir -p "$SALIDA"

# --no-dependencies: no hay ninguna, y
# sin la opcion vsce busca un
# package-lock.json que no existe.
( cd "$AQUI" && npx --yes @vscode/vsce package \
    --no-dependencies \
    --out "$SALIDA/tzlang-$VERSION.vsix" )

echo
echo "Listo: $SALIDA/tzlang-$VERSION.vsix"
echo
echo "Para instalarlo:"
echo
echo "  code --install-extension $SALIDA/tzlang-$VERSION.vsix"
echo
echo "Y recarga la ventana de VS Code para verlo."
