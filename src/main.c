#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * tzc <archivo.tz>
 */

int main(int argc, char **argv) {

    /*
     * Ni de menos ni de mas: un
     * unico archivo.
     */

    if (argc != 2) {

        fprintf(
            stderr,
            "Uso: tzc <archivo.tz>\n"
        );

        return SALIDA_ERROR_ARGUMENTOS;
    }

    const char *path = argv[1];

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
     * El buffer se libera SIEMPRE,
     * haya ido bien o mal.
     */

    free(source);

    return
        ok
            ? SALIDA_OK
            : SALIDA_ERROR_PROGRAMA;
}
