#!/bin/sh
#
# Suite educativa de TzLang.
#
# Valida el material de education/. Es una suite
# APARTE: no sustituye ni toca tests/run_tests.sh.
#
# Ejecuta el binario REAL sobre cada leccion .tz.
# No enlaza ni llama a funciones internas de C, y
# no duplica nada del lenguaje.
#
# Una leccion es:
#
#   nombre.tz         el programa
#   nombre.expected   la salida esperada (stdout)
#
# Se comprueba, para cada leccion:
#
#   1. que el codigo de salida sea 0
#   2. que stderr este vacio
#   3. que stdout sea exactamente el .expected
#
# A diferencia de la suite principal, aqui stdout y
# stderr NO se concatenan: el material educativo no
# debe producir errores, asi que cualquier cosa en
# stderr es un fallo por si misma.
#
# El binario se puede elegir mediante TZC, igual que
# en la suite principal:
#
#   TZC=build/tzc-asan tests/run_education_tests.sh
#
# Salida: 0 si todas pasan, 1 si alguna falla.

set -u

# ------------------------------------------
# Localizar el proyecto desde este script
# ------------------------------------------
#
# Las rutas se resuelven desde la ubicacion del
# script y no desde el directorio actual, para que
# la suite funcione se invoque desde donde se
# invoque.

SCRIPT_DIR=$(dirname "$0")
ROOT=$(cd "$SCRIPT_DIR/.." && pwd)

TZC="${TZC:-$ROOT/build/tzc}"
EDU_DIR="$ROOT/education"

if [ ! -x "$TZC" ]; then
    printf 'Error: no se encontro %s\n' "$TZC"
    printf 'Compila primero con: make\n'
    exit 1
fi

if [ ! -d "$EDU_DIR" ]; then
    printf 'Error: no se encontro el directorio %s\n' "$EDU_DIR"
    exit 1
fi

TMP=$(mktemp -d 2>/dev/null || mktemp -d -t tzlang)
trap 'rm -rf "$TMP"' EXIT INT TERM

TOTAL=0
PASSED=0
FAILED=0

# ------------------------------------------
# report_failure <nombre> <motivo> <exit real>
# ------------------------------------------

report_failure() {
    name=$1
    reason=$2
    got_code=$3

    printf '[FAIL] %s\n' "$name"
    printf '  %s\n\n' "$reason"

    printf 'Expected:\n'
    sed 's/^/  /' "$TMP/expected"

    printf '\nActual:\n'
    sed 's/^/  /' "$TMP/out"

    printf '\nDiff:\n'
    diff "$TMP/expected" "$TMP/out" 2>&1 | sed 's/^/  /'

    if [ -s "$TMP/err" ]; then
        printf '\nStderr:\n'
        sed 's/^/  /' "$TMP/err"
    fi

    printf '\nExit code:\n'
    printf '  Expected: 0\n'
    printf '  Actual:   %s\n\n' "$got_code"
}

# ------------------------------------------
# Recorrer archivos con seguridad
# ------------------------------------------
#
# 'for x in $(find ...)' parte las rutas por
# espacios, asi que no se usa. La lista se vuelca a
# un archivo y se lee linea a linea:
#
#   IFS=      no recorta espacios al principio ni
#             al final del nombre
#   read -r   no interpreta las barras invertidas
#
# El bucle recibe la lista por REDIRECCION y no por
# tuberia: una tuberia ejecutaria el while en una
# subshell y los contadores TOTAL/PASSED/FAILED se
# perderian al terminar.
#
# Esto sostiene rutas con espacios y nombres con
# caracteres Unicode.
#
# Limitacion asumida, la misma que la suite
# principal: un nombre de archivo que contenga un
# salto de linea no se soporta. Distinguirlo
# exigiria -print0 / -d '', que NO son POSIX.

run_education_tests() {

    find "$EDU_DIR" -name '*.tz' | sort > "$TMP/lista_edu"

    while IFS= read -r tz; do

        base=${tz%.tz}
        name=${base#"$EDU_DIR"/}

        expected="$base.expected"

        TOTAL=$((TOTAL + 1))

        if [ ! -f "$expected" ]; then
            FAILED=$((FAILED + 1))
            printf '[FAIL] %s\n' "$name"
            printf '  falta el archivo %s\n\n' "$expected"
            continue
        fi

        # stdout y stderr por separado: aqui NO se
        # concatenan, se juzgan por separado.
        "$TZC" "$tz" >"$TMP/out" 2>"$TMP/err"
        got_code=$?

        cp "$expected" "$TMP/expected"

        if [ "$got_code" -ne 0 ]; then
            FAILED=$((FAILED + 1))
            report_failure "$name" "el programa termino con error" "$got_code"
            continue
        fi

        if [ -s "$TMP/err" ]; then
            FAILED=$((FAILED + 1))
            report_failure "$name" "el programa escribio en stderr" "$got_code"
            continue
        fi

        if diff -q "$TMP/expected" "$TMP/out" >/dev/null 2>&1; then
            PASSED=$((PASSED + 1))
            printf '[PASS] %s\n' "$name"
        else
            FAILED=$((FAILED + 1))
            report_failure "$name" "la salida no coincide con lo esperado" "$got_code"
        fi

    done < "$TMP/lista_edu"
}

# ------------------------------------------

printf '=== TzLang Education Suite ===\n\n'

run_education_tests

if [ "$TOTAL" -eq 0 ]; then
    printf 'Error: no se encontro ningun programa .tz en %s\n' "$EDU_DIR"
    exit 1
fi

printf '\n========================================\n'
printf 'Tests:  %d\n' "$TOTAL"
printf 'Passed: %d\n' "$PASSED"
printf 'Failed: %d\n' "$FAILED"
printf '========================================\n\n'

if [ "$FAILED" -eq 0 ]; then
    printf 'All education tests passed.\n'
    exit 0
fi

printf '%d education test(s) failed.\n' "$FAILED"
exit 1
