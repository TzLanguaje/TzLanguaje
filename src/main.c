#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"

int main(void) {

const char *source =
    "// PRUEBA 1\n"
    "variable contador = 0\n"
    "mientras (contador es menor que 5)\n"
    "    imprimir contador\n"
    "    contador = contador + 1\n"
    "fin\n"
    "\n"
    "// PRUEBA 2\n"
    "variable c2 = 0\n"
    "mientras (c2 es menor que 3)\n"
    "    c2 = c2 + 1\n"
    "fin\n"
    "imprimir c2\n"
    "\n"
    "// PRUEBA 3\n"
    "mientras (falso)\n"
    "    imprimir \"ERROR\"\n"
    "fin\n"
    "\n"
    "// PRUEBA 4\n"
    "variable c4 = 0\n"
    "mientras (c4 es menor que 3)\n"
    "\n"
    "    si (c4 es igual a 1)\n"
    "        imprimir \"Uno\"\n"
    "    fin\n"
    "\n"
    "    c4 = c4 + 1\n"
    "fin\n"
    "\n"
    "// PRUEBA 5\n"
    "variable numero = 0\n"
    "mientras (numero + 1 * 2 es menor que 10)\n"
    "    numero = numero + 1\n"
    "fin\n"
    "imprimir numero\n";
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