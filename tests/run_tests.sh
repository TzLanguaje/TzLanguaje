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
#   nombre.notas      si existe, el test se ejecuta con TZ_NOTAS=1, es decir
#                     con las notas de diagnostico activadas (opcional)
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

# El binario a probar. Se puede apuntar a otro
# (por ejemplo el instrumentado con ASan):
#
#   TZC=build/tzc-asan tests/run_tests.sh
#
# Sin variable, el normal.
TZC="${TZC:-$ROOT/build/tzc}"
TESTS_DIR="$ROOT/tests"

# La version sale de src/version.h, no
# escrita a mano en los .expected: si
# no, subir la version romperia estos
# tests en cada release.
VERSION=$(sed -n 's/.*TZLANG_VERSION "\([0-9.]*\)".*/\1/p' "$ROOT/src/version.h")

# Copia una expectativa sustituyendo
# @VERSION@ por la version real.
expectativa_con_version() {
    sed "s/@VERSION@/$VERSION/g" "$1" > "$TMP/expected"
}

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
# Recorrer archivos con seguridad
# ------------------------------------------
#
# 'for x in $(find ...)' parte las rutas por
# espacios, asi que no se usa.
#
# En su lugar la lista se vuelca a un archivo
# y se lee linea a linea:
#
#   IFS=      no recorta espacios al principio
#             ni al final del nombre
#   read -r   no interpreta las barras
#             invertidas
#
# El bucle recibe la lista por REDIRECCION y
# no por tuberia. Es importante: una tuberia
# ejecutaria el while en una subshell y los
# contadores TOTAL/PASSED/FAILED se perderian
# al terminar.
#
# Limitacion asumida: un nombre de archivo que
# contenga un salto de linea no se soporta.
# Distinguirlo exigiria -print0 / -d '', que
# NO son POSIX.

# ------------------------------------------
# Tests basados en archivos .tz
# ------------------------------------------

run_file_tests() {

    find "$TESTS_DIR" -name '*.tz' | sort > "$TMP/lista_tests"

    while IFS= read -r tz; do

        base=${tz%.tz}
        name=${base#"$TESTS_DIR"/}

        expected="$base.expected"
        exitfile="$base.exit"

        # Las notas de diagnostico van
        # apagadas salvo que el test
        # pida lo contrario. Asi los
        # tests de siempre siguen
        # comparando exactamente la
        # misma salida que antes.
        notasfile="$base.notas"

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
        if [ -f "$notasfile" ]; then
            TZ_NOTAS=1 "$TZC" "$tz" >"$TMP/out" 2>"$TMP/err"
        else
            TZ_NOTAS=0 "$TZC" "$tz" >"$TMP/out" 2>"$TMP/err"
        fi
        got_code=$?

        cat "$TMP/out" "$TMP/err" > "$TMP/actual"
        cp "$expected" "$TMP/expected"

        check "$name" "$want_code" "$got_code"

    done < "$TMP/lista_tests"
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
    expectativa_con_version "$TESTS_DIR/cli/help.expected"
    cli_case "cli/help_long" 0 --help

    expectativa_con_version "$TESTS_DIR/cli/help.expected"
    cli_case "cli/help_short" 0 -h

    expectativa_con_version "$TESTS_DIR/cli/version.expected"
    cli_case "cli/version_long" 0 --version

    expectativa_con_version "$TESTS_DIR/cli/version.expected"
    cli_case "cli/version_short" 0 -v

    # opcion desconocida: no debe tratarse como archivo
    printf "Error: opción desconocida '--banana'.\n\nUso: tz <archivo.tz>\n" > "$TMP/expected"
    cli_case "cli/unknown_option_long" 1 --banana

    printf "Error: opción desconocida '-x'.\n\nUso: tz <archivo.tz>\n" > "$TMP/expected"
    cli_case "cli/unknown_option_short" 1 -x

    # --- argumentos ---

    # sin argumentos
    printf 'Uso: tz <archivo.tz>\n' > "$TMP/expected"
    cli_case "cli/no_arguments" 1

    # demasiados argumentos
    printf 'Uso: tz <archivo.tz>\n' > "$TMP/expected"
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
# Notas de diagnostico
# ------------------------------------------
#
# Comprueban lo que los .expected no pueden:
# que la nota sea DETERMINISTA y que este
# apagada por defecto cuando la salida no es
# un terminal.

entrada_tests() {

    # entrada() lee de la entrada estandar, asi
    # que no puede ser un test de archivo suelto:
    # el runner no le pasaria nada y quedaria a
    # merced de lo que hubiera en stdin.

    programa="$TMP/entrada.tz"

    cat > "$programa" <<'PROGRAMA'
variable nombre = entrada("Nombre: ")
imprimir "Hola " + nombre
variable edad = numero(entrada("Edad: "))
imprimir texto(edad + 1)
PROGRAMA

    TOTAL=$((TOTAL + 1))

    printf 'Ana\n30\n' | "$TZC" "$programa" > "$TMP/e_out" 2>&1

    printf 'Nombre: Hola Ana\nEdad: 31\n' > "$TMP/e_esperado"

    if diff -q "$TMP/e_esperado" "$TMP/e_out" >/dev/null 2>&1; then
        PASSED=$((PASSED + 1))
        printf '[PASS] %s\n' "entrada/lee_una_linea"
    else
        FAILED=$((FAILED + 1))
        printf '[FAIL] %s\n' "entrada/lee_una_linea"
        diff "$TMP/e_esperado" "$TMP/e_out" | sed 's/^/  /'
    fi

    # Sin nada que leer devuelve texto vacio,
    # no falla: en un CI la entrada esta cerrada.

    TOTAL=$((TOTAL + 1))

    cat > "$programa" <<'PROGRAMA'
variable x = entrada("")
imprimir "[" + x + "]"
imprimir largo(x)
PROGRAMA

    "$TZC" "$programa" < /dev/null > "$TMP/e_out2" 2>&1

    if [ "$(cat "$TMP/e_out2")" = "[]
0" ]; then
        PASSED=$((PASSED + 1))
        printf '[PASS] %s\n' "entrada/sin_entrada_texto_vacio"
    else
        FAILED=$((FAILED + 1))
        printf '[FAIL] %s\n' "entrada/sin_entrada_texto_vacio"
        cat "$TMP/e_out2" | sed 's/^/  /'
    fi
}

diagnostic_tests() {

    programa="$TESTS_DIR/diagnostics/identificador.tz"

    # --- determinismo ---
    #
    # El mismo error, tres veces seguidas,
    # tiene que dar exactamente la misma
    # salida. Si algun dia se introduce azar
    # al elegir la frase, esto lo caza.

    TOTAL=$((TOTAL + 1))

    TZ_NOTAS=1 "$TZC" "$programa" >"$TMP/d1" 2>&1
    TZ_NOTAS=1 "$TZC" "$programa" >"$TMP/d2" 2>&1
    TZ_NOTAS=1 "$TZC" "$programa" >"$TMP/d3" 2>&1

    if diff -q "$TMP/d1" "$TMP/d2" >/dev/null 2>&1 &&
       diff -q "$TMP/d2" "$TMP/d3" >/dev/null 2>&1
    then
        PASSED=$((PASSED + 1))
        printf '[PASS] %s\n' "diagnostics/determinista"
    else
        FAILED=$((FAILED + 1))
        printf '[FAIL] %s\n' "diagnostics/determinista"
        printf '  la nota cambia entre ejecuciones identicas\n\n'
    fi

    # --- apagadas por defecto ---
    #
    # Sin TZ_NOTAS y con la salida redirigida a
    # un archivo, no debe aparecer ninguna nota:
    # es lo que mantiene intacta la salida para
    # los scripts que ya existen.

    TOTAL=$((TOTAL + 1))

    unset TZ_NOTAS
    "$TZC" "$programa" >"$TMP/sin" 2>&1

    if grep -q "Error: variable 'noExiste' no existe." "$TMP/sin" &&
       ! grep -q "Ese nombre todavía no existe" "$TMP/sin"
    then
        PASSED=$((PASSED + 1))
        printf '[PASS] %s\n' "diagnostics/apagadas_por_defecto"
    else
        FAILED=$((FAILED + 1))
        printf '[FAIL] %s\n' "diagnostics/apagadas_por_defecto"
        printf '  la nota aparece con la salida redirigida\n\n'
    fi

    # --- el tecnico nunca desaparece ---
    #
    # Con las notas ENCENDIDAS, el diagnostico
    # de siempre tiene que seguir ahi.

    TOTAL=$((TOTAL + 1))

    TZ_NOTAS=1 "$TZC" "$programa" >"$TMP/con" 2>&1

    if grep -q "Error: variable 'noExiste' no existe." "$TMP/con" &&
       grep -q "La ejecución falló." "$TMP/con" &&
       grep -q "Ese nombre todavía no existe" "$TMP/con"
    then
        PASSED=$((PASSED + 1))
        printf '[PASS] %s\n' "diagnostics/tecnico_preservado"
    else
        FAILED=$((FAILED + 1))
        printf '[FAIL] %s\n' "diagnostics/tecnico_preservado"
        printf '  falta el diagnostico tecnico o la nota\n\n'
    fi
}

# ------------------------------------------
# Validacion de examples/
#
# No comprueba la salida: solo que cada
# ejemplo se ejecute sin error.
# ------------------------------------------

example_tests() {

    find "$ROOT/examples" -name '*.tz' | sort > "$TMP/lista_examples"

    while IFS= read -r tz; do

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

    done < "$TMP/lista_examples"
}

# ------------------------------------------

printf '=== TzLang Test Suite ===\n\n'

run_file_tests
cli_extra_tests
diagnostic_tests
entrada_tests
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
