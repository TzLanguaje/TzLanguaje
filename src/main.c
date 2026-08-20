#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "version.h"

#include "diagnostic/diagnostic.h"

#include "io/console.h"
#include "io/file.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"

/*
 * ==========================
 * CODIGOS DE SALIDA
 * ==========================
 *
 * 0              todo fue bien
 * distinto de 0  hubo un error
 *
 * Se distinguen las tres familias
 * para que un script pueda saber
 * QUE fallo.
 */

#define SALIDA_OK                0
#define SALIDA_ERROR_ARGUMENTOS  1
#define SALIDA_ERROR_ARCHIVO     2
#define SALIDA_ERROR_PROGRAMA    3

/*
 * ==========================
 * AYUDA Y VERSION
 * ==========================
 *
 * Van a stdout porque son SALIDA
 * pedida por el usuario, no
 * errores. Los mensajes de uso por
 * argumentos invalidos van a
 * stderr.
 */

static void mostrar_version(void) {

    printf(
        "%s %s\n",
        TZLANG_NAME,
        TZLANG_VERSION
    );
}

static void mostrar_ayuda(void) {

    mostrar_version();

    printf(
        "\n"
        "Uso:\n"
        "  tz <archivo.tz>\n"
        "\n"
        "Opciones:\n"
        "  -h, --help       Mostrar esta ayuda\n"
        "  -v, --version    Mostrar la versión\n"
        "\n"
        "Ejemplos:\n"
        "  tz programa.tz\n"
        "  tz examples/hola.tz\n"
    );
}

static void mostrar_uso(void) {

    fprintf(
        stderr,
        "Uso: tz <archivo.tz>\n"
    );
}

/*
 * ¿El argumento es esta opcion, en
 * su forma corta o larga?
 */

static int es_opcion(
    const char *argumento,
    const char *corta,
    const char *larga
) {

    return
        strcmp(argumento, corta) == 0 ||
        strcmp(argumento, larga) == 0;
}

/*
 * ==========================
 * EXTENSION
 * ==========================
 *
 * La validacion vive aqui, en la
 * entrada de archivos, NO en el
 * lexer: el lenguaje no sabe nada
 * de nombres de archivo.
 */

static int tiene_extension_tz(
    const char *path
) {

    size_t length = strlen(path);

    if (length < 3) {
        return 0;
    }

    return
        strcmp(path + length - 3, ".tz") == 0;
}

/*
 * ==========================
 * PIPELINE
 * ==========================
 *
 * Exactamente el mismo flujo de
 * siempre:
 *
 * lexer -> parser -> AST -> interpreter
 *
 * Lo unico que cambio es de donde
 * sale 'source'.
 *
 * Devuelve 1 si el programa se
 * ejecuto entero, 0 si fallo en
 * cualquier etapa.
 */

static int ejecutar_fuente(
    const char *source
) {

    /*
     * LEXER
     */

    int token_count = 0;

    Token *tokens =
        lexer_tokenize(
            source,
            &token_count
        );

    if (tokens == NULL) {

        fprintf(
            stderr,
            "Error ejecutando Lexer.\n"
        );

        return 0;
    }

    /*
     * PARSER
     */

    Parser *parser =
        parser_create(
            tokens,
            token_count
        );

    if (parser == NULL) {

        fprintf(
            stderr,
            "Error creando Parser.\n"
        );

        lexer_free_tokens(
            tokens,
            token_count
        );

        return 0;
    }

    /*
     * AST
     */

    ASTNode *program =
        parser_parse(parser);

    if (program == NULL) {

        fprintf(
            stderr,
            "Error construyendo AST.\n"
        );

        parser_free(parser);
        lexer_free_tokens(tokens, token_count);

        return 0;
    }

    /*
     * INTERPRETER
     */

    if (!interpreter_run(program)) {

        /*
         * Red de seguridad: si el
         * error concreto no registro
         * su categoria, queda la
         * general de ejecucion.
         * diagnostic_registrar()
         * respeta la primera, asi que
         * esto nunca pisa una mas
         * precisa.
         */

        diagnostic_registrar(DIAG_SEMANTICO);

        fprintf(
            stderr,
            "La ejecución falló.\n"
        );

        ast_free(program);
        parser_free(parser);
        lexer_free_tokens(tokens, token_count);

        return 0;
    }

    /*
     * LIMPIEZA
     */

    ast_free(program);

    parser_free(parser);

    lexer_free_tokens(
        tokens,
        token_count
    );

    return 1;
}

/*
 * ==========================
 * ENTRADA
 * ==========================
 *
 * tz <archivo.tz>
 */

int main(int argc, char **argv) {

    /*
     * Lo PRIMERO, antes de imprimir
     * nada: si no, el primer mensaje
     * ya saldria roto.
     */

    console_init();

    /*
     * Ni de menos ni de mas: un
     * unico archivo.
     */

    if (argc != 2) {

        mostrar_uso();

        return SALIDA_ERROR_ARGUMENTOS;
    }

    const char *argumento = argv[1];

    /*
     * ==========================
     * OPCIONES
     * ==========================
     */

    if (es_opcion(argumento, "-h", "--help")) {

        mostrar_ayuda();

        return SALIDA_OK;
    }

    if (es_opcion(argumento, "-v", "--version")) {

        mostrar_version();

        return SALIDA_OK;
    }

    /*
     * Todo lo que empieza por '-' y
     * no reconocemos es una opcion
     * equivocada, NO un nombre de
     * archivo: asi '--banana' no
     * acaba dando "no es un archivo
     * .tz", que despistaria.
     */

    if (argumento[0] == '-') {

        fprintf(
            stderr,
            "Error: opción desconocida '%s'.\n\n",
            argumento
        );

        mostrar_uso();

        return SALIDA_ERROR_ARGUMENTOS;
    }

    const char *path = argumento;

    if (!tiene_extension_tz(path)) {

        fprintf(
            stderr,
            "Error: '%s' no es un archivo .tz.\n",
            path
        );

        return SALIDA_ERROR_ARCHIVO;
    }

    char *source = read_file(path);

    if (source == NULL) {

        fprintf(
            stderr,
            "Error: no se pudo abrir el archivo '%s'.\n",
            path
        );

        return SALIDA_ERROR_ARCHIVO;
    }

    int ok = ejecutar_fuente(source);

    /*
     * La nota va DESPUES de todos los
     * mensajes tecnicos, una sola vez
     * y solo si algo fallo.
     */

    if (!ok) {
        diagnostic_imprimir_nota();
    }

    /*
     * El buffer se libera SIEMPRE,
     * haya ido bien o mal.
     */

    free(source);

    return
        ok
            ? SALIDA_OK
            : SALIDA_ERROR_PROGRAMA;
}
