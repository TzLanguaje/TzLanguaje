#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"

int main(void) {

    const char *source =
        "variable x = 10;\n"
        "variable y = 20;\n"
        "imprimir(x + y);";

    /*
     * ==========================
     * LEXER
     * ==========================
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
            "Error: no se pudieron crear los tokens.\n"
        );

        return 1;
    }

    /*
     * ==========================
     * PARSER
     * ==========================
     */

    Parser *parser =
        parser_create(
            tokens,
            token_count
        );

    if (parser == NULL) {

        fprintf(
            stderr,
            "Error: no se pudo crear el Parser.\n"
        );

        lexer_free_tokens(
            tokens,
            token_count
        );

        return 1;
    }

    ASTNode *program =
        parser_parse(parser);

    if (program == NULL) {

        fprintf(
            stderr,
            "Error: no se pudo construir el AST.\n"
        );

        parser_free(parser);

        lexer_free_tokens(
            tokens,
            token_count
        );

        return 1;
    }

    /*
     * ==========================
     * EJECUTAR
     * ==========================
     */

    printf("=== TzLang ===\n\n");

    if (
        !interpreter_run(program)
    ) {

        fprintf(
            stderr,
            "\nLa ejecución falló.\n"
        );

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
     * LIMPIEZA
     * ==========================
     */

    ast_free(program);

    parser_free(parser);

    lexer_free_tokens(
        tokens,
        token_count
    );

    return 0;
}