#!/bin/sh
#
# Suite de tests de TzLang.
#
# Ejecuta el binario REAL sobre archivos .tz.
# No enlaza ni llama a funciones internas de C.
#
# Un test es:
#
#   nombre.tz         el programa
#   nombre.expected   la salida esperada (stdout + stderr)
#   nombre.exit       el codigo de salida esperado (opcional, por defecto 0)
#
# Los tests del CLI que no son archivos .tz viven
# al final, en cli_extra_tests().
#
# Salida: 0 si todos pasan, 1 si alguno falla.

set -u

# ------------------------------------------
# Localizar el proyecto desde este script
# ------------------------------------------

SCRIPT_DIR=$(dirname "$0")
ROOT=$(cd "$SCRIPT_DIR/.." && pwd)

TZC="$ROOT/build/tzc"
TESTS_DIR="$ROOT/tests"

if [ ! -x "$TZC" ]; then
    printf 'Error: no se encontro %s\n' "$TZC"
    printf 'Compila primero con: make\n'
    exit 1
fi

TMP=$(mktemp -d 2>/dev/null || mktemp -d -t tzlang)
trap 'rm -rf "$TMP"' EXIT INT TERM

TOTAL=0
PASSED=0
FAILED=0

# ------------------------------------------
# report_failure <nombre> <exit esperado> <exit real>
#
# Muestra el diff y los codigos de salida.
# ------------------------------------------

report_failure() {
    name=$1
    want_code=$2
    got_code=$3

    printf '[FAIL] %s\n\n' "$name"

    printf 'Expected:\n'
    cat "$TMP/expected"
    printf '\nActual:\n'
    cat "$TMP/actual"

    printf '\nDiff:\n'
    diff "$TMP/expected" "$TMP/actual" 2>&1 | sed 's/^/  /'

    printf '\nExit code:\n'
    printf '  Expected: %s\n' "$want_code"
    printf '  Actual:   %s\n\n' "$got_code"
}

# ------------------------------------------
# check <nombre> <exit esperado> <exit real>
#
# Compara $TMP/expected con $TMP/actual.
# ------------------------------------------

check() {
    name=$1
    want_code=$2
    got_code=$3

    TOTAL=$((TOTAL + 1))

    if [ "$want_code" = "$got_code" ] &&
       diff -q "$TMP/expected" "$TMP/actual" >/dev/null 2>&1
    then
        PASSED=$((PASSED + 1))
        printf '[PASS] %s\n' "$name"
    else
        FAILED=$((FAILED + 1))
        report_failure "$name" "$want_code" "$got_code"
    fi
}

# ------------------------------------------
# Tests basados en archivos .tz
# ------------------------------------------

run_file_tests() {
    for tz in $(find "$TESTS_DIR" -name '*.tz' | sort); do

        base=${tz%.tz}
        name=${base#"$TESTS_DIR"/}

        expected="$base.expected"
        exitfile="$base.exit"

        if [ ! -f "$expected" ]; then
            TOTAL=$((TOTAL + 1))
            FAILED=$((FAILED + 1))
            printf '[FAIL] %s\n' "$name"
            printf '  falta el archivo %s\n\n' "$expected"
            continue
        fi

        if [ -f "$exitfile" ]; then
            want_code=$(cat "$exitfile" | tr -d ' \n\r')
        else
            want_code=0
        fi

        # stdout y stderr por separado y luego concatenados,
        # para que el orden no dependa del buffering.
        "$TZC" "$tz" >"$TMP/out" 2>"$TMP/err"
        got_code=$?

        cat "$TMP/out" "$TMP/err" > "$TMP/actual"
        cp "$expected" "$TMP/expected"

        check "$name" "$want_code" "$got_code"
    done
}

# ------------------------------------------
# Tests del CLI que no son archivos .tz
# ------------------------------------------

cli_case() {
    name=$1
    want_code=$2
    shift 2

    "$TZC" "$@" >"$TMP/out" 2>"$TMP/err"
    got_code=$?

    cat "$TMP/out" "$TMP/err" > "$TMP/actual"

    check "$name" "$want_code" "$got_code"
}

cli_extra_tests() {
    # --- opciones ---

    # La ayuda y la version son multilinea, asi que la
    # expectativa vive en un archivo y no duplicada aqui.
    cp "$TESTS_DIR/cli/help.expected" "$TMP/expected"
    cli_case "cli/help_long" 0 --help

    cp "$TESTS_DIR/cli/help.expected" "$TMP/expected"
    cli_case "cli/help_short" 0 -h

    cp "$TESTS_DIR/cli/version.expected" "$TMP/expected"
    cli_case "cli/version_long" 0 --version

    cp "$TESTS_DIR/cli/version.expected" "$TMP/expected"
    cli_case "cli/version_short" 0 -v

    # opcion desconocida: no debe tratarse como archivo
    printf "Error: opción desconocida '--banana'.\n\nUso: tzc <archivo.tz>\n" > "$TMP/expected"
    cli_case "cli/unknown_option_long" 1 --banana

    printf "Error: opción desconocida '-x'.\n\nUso: tzc <archivo.tz>\n" > "$TMP/expected"
    cli_case "cli/unknown_option_short" 1 -x

    # --- argumentos ---

    # sin argumentos
    printf 'Uso: tzc <archivo.tz>\n' > "$TMP/expected"
    cli_case "cli/no_arguments" 1

    # demasiados argumentos
    printf 'Uso: tzc <archivo.tz>\n' > "$TMP/expected"
    cli_case "cli/too_many_arguments" 1 "$TESTS_DIR/cli/crlf.tz" "$TESTS_DIR/cli/crlf.tz"

    # archivo inexistente
    printf "Error: no se pudo abrir el archivo 'no_existe.tz'.\n" > "$TMP/expected"
    cli_case "cli/missing_file" 2 "no_existe.tz"

    # extension incorrecta
    printf "Error: 'programa.txt' no es un archivo .tz.\n" > "$TMP/expected"
    cli_case "cli/bad_extension" 2 "programa.txt"

    # un directorio no es un .tz
    printf "Error: '%s' no es un archivo .tz.\n" "$TESTS_DIR" > "$TMP/expected"
    cli_case "cli/directory" 2 "$TESTS_DIR"

    # ejecucion normal de un .tz desde la ruta del proyecto
    printf 'Hola desde TzLang\n' > "$TMP/expected"
    cli_case "cli/run_example" 0 "$ROOT/examples/hola.tz"
}

# ------------------------------------------
# Validacion de examples/
#
# No comprueba la salida: solo que cada
# ejemplo se ejecute sin error.
# ------------------------------------------

example_tests() {
    for tz in $(find "$ROOT/examples" -name '*.tz' | sort); do

        name="examples/$(basename "$tz")"

        "$TZC" "$tz" >"$TMP/out" 2>"$TMP/err"
        got_code=$?

        TOTAL=$((TOTAL + 1))

        if [ "$got_code" = "0" ]; then
            PASSED=$((PASSED + 1))
            printf '[PASS] %s\n' "$name"
        else
            FAILED=$((FAILED + 1))
            printf '[FAIL] %s\n' "$name"
            printf '  exit code %s (se esperaba 0)\n' "$got_code"
            sed 's/^/  /' "$TMP/err"
            printf '\n'
        fi
    done
}

# ------------------------------------------

printf '=== TzLang Test Suite ===\n\n'

run_file_tests
cli_extra_tests
example_tests

printf '\n========================================\n'
printf 'Tests:  %d\n' "$TOTAL"
printf 'Passed: %d\n' "$PASSED"
printf 'Failed: %d\n' "$FAILED"
printf '========================================\n\n'

if [ "$FAILED" -eq 0 ]; then
    printf 'All tests passed.\n'
    exit 0
fi

printf '%d test(s) failed.\n' "$FAILED"
exit 1
