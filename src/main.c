#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"

int main(void) {

const char *source =
    "imprimir(10 + 20);\n"
    "imprimir(10 - 3);\n"
    "imprimir(5 * 4);\n"
    "imprimir(20 / 5);\n"
    "imprimir(10.5 + 2.5);\n"
    "imprimir(10.5 - 2.5);\n"
    "imprimir(5 * 2.5);\n"
    "imprimir(10 / 2.5);\n"
    "imprimir(\"Hola \" + \"TzLang\");";
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

        return 1;
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

        return 1;
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

        return 1;
    }

    /*
     * INTERPRETER
     */

    printf("=== TzLang ===\n\n");

    if (!interpreter_run(program)) {

        fprintf(
            stderr,
            "La ejecución falló.\n"
        );

        ast_free(program);
        parser_free(parser);
        lexer_free_tokens(tokens, token_count);

        return 1;
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

    return 0;
}