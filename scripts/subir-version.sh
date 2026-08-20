#!/bin/sh
#
# ==========================
# Sube la version de TzLang
# ==========================
#
#   sh scripts/subir-version.sh 0.3.0
#
# El numero vive en siete archivos.
# Cambiarlos a mano es como se olvida
# uno, y entonces el binario dice una
# version y el README otra.
#
# El script cambia los siete, recompila
# y pasa la suite. NO hace commit ni
# etiqueta: eso lo decides tu, al
# final te imprime las ordenes.

set -eu

NUEVA="${1:-}"

if [ -z "$NUEVA" ]; then
    echo "Uso: sh scripts/subir-version.sh X.Y.Z" >&2
    exit 1
fi

# Formato: tres numeros. Sin 'v'
# delante: la 'v' es solo de la
# etiqueta de git.
if ! echo "$NUEVA" | grep -Eq '^[0-9]+\.[0-9]+\.[0-9]+$'; then
    echo "Error: '$NUEVA' no tiene la forma X.Y.Z (sin la 'v')" >&2
    exit 1
fi

RAIZ="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
cd "$RAIZ"

ACTUAL="$(sed -n 's/.*TZLANG_VERSION "\([0-9.]*\)".*/\1/p' src/version.h)"

if [ "$ACTUAL" = "$NUEVA" ]; then
    echo "La version ya es $NUEVA. No hay nada que hacer."
    exit 0
fi

# El arbol tiene que estar limpio: si
# no, no se distingue lo que cambio el
# script de lo que ya habia.
if [ -n "$(git status --porcelain)" ]; then
    echo "Error: hay cambios sin guardar." >&2
    echo "Haz commit o guardalos antes de subir la version." >&2
    exit 1
fi

echo "TzLang $ACTUAL -> $NUEVA"
echo

# ==========================
# LOS SIETE ARCHIVOS
# ==========================
#
# version.h es la fuente de verdad; el
# resto son menciones que tienen que
# seguirla.

ARCHIVOS="src/version.h README.md docs/language.md install.sh install.ps1 packaging/homebrew/tzlang.rb packaging/scoop/tzlang.json"

for f in $ARCHIVOS; do
    if grep -q "$ACTUAL" "$f" 2>/dev/null; then
        sed -i.bak "s/$ACTUAL/$NUEVA/g" "$f" && rm -f "$f.bak"
        echo "   actualizado  $f"
    else
        echo "   sin cambios  $f"
    fi
done

echo
echo "Comprobando que no quede ninguna mencion vieja..."

if grep -rn "$ACTUAL" $ARCHIVOS 2>/dev/null; then
    echo "AVISO: quedan menciones a $ACTUAL, revisalas a mano." >&2
else
    echo "   ninguna"
fi

# ==========================
# COMPILAR Y PROBAR
# ==========================
#
# Que la version nueva no rompa nada.
# Las expectativas de los tests usan
# @VERSION@, asi que deberian pasar
# solas.

echo
echo "Compilando..."
make clean >/dev/null
make >/dev/null 2>&1 || { echo "Error: no compila" >&2; exit 1; }

echo "   binario: $(./build/tzc --version)"

echo
echo "Pasando la suite..."
if ! ./tests/run_tests.sh > /tmp/tz-suite.log 2>&1; then
    echo "Error: la suite NO pasa. Nada se ha commiteado." >&2
    grep -E "^\[FAIL\]|Failed:" /tmp/tz-suite.log >&2
    exit 1
fi
grep -E "Tests:|Passed:|Failed:" /tmp/tz-suite.log | sed 's/^/   /'

if ! ./tests/run_education_tests.sh > /tmp/tz-edu.log 2>&1; then
    echo "Error: la suite de education NO pasa." >&2
    exit 1
fi
echo "   education: pasa"

# ==========================
# QUE HACER AHORA
# ==========================

cat <<FIN

Listo. Nada commiteado todavia.

Revisa el cambio:

  git diff

Y cuando estes conforme:

  git add -A
  git commit -m "chore: version $NUEVA"
  git push

  git tag v$NUEVA
  git push origin v$NUEVA

Ese ultimo push dispara el release. La
guarda del workflow comprueba que la
etiqueta y src/version.h coincidan, asi
que si algo quedo desincronizado se
para antes de publicar nada.
FIN
