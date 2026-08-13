#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"

int main(void) {

const char *source =
    "variable edad = 20\n"
    "edad = 25\n"
    "imprimir edad\n"
    "\n"
    "variable contador = 0\n"
    "contador = contador + 1\n"
    "contador = contador + 1\n"
    "contador = contador + 1\n"
    "imprimir contador\n"
    "\n"
    "variable numero = 5\n"
    "numero = numero * 2\n"
    "imprimir numero\n"
    "\n"
    "variable edad2 = 20\n"
    "variable incremento = 5\n"
    "edad2 = edad2 + incremento\n"
    "imprimir edad2\n"
    "\n"
    "variable altura = 1.5\n"
    "altura = altura + 0.25\n"
    "imprimir altura\n";
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