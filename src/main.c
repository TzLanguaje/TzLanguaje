#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"

int main(void) {

const char *source =
    "imprimir -5\n"
    "imprimir -3.14\n"
    "imprimir -(5 + 3)\n"
    "imprimir -5 + 10\n"
    "imprimir 2 * -4\n"
    "imprimir -5 * 2\n"
    "imprimir absoluto(-5)\n"
    "\n"
    "variable x = -10\n"
    "imprimir x\n"
    "imprimir -x\n"
    "\n"
    "variable lista = [-1, -2, 3]\n"
    "imprimir lista\n"
    "\n"
    "funcion doble(v)\n"
    "    retornar v * 2\n"
    "fin\n"
    "imprimir doble(-5)\n"
    "\n"
    "funcion neg()\n"
    "    retornar -10\n"
    "fin\n"
    "imprimir neg()\n"
    "\n"
    "imprimir -5 es menor que 0\n"
    "imprimir --5\n"
    "\n"
    "// la resta binaria no cambia\n"
    "imprimir 10 - 3\n"
    "imprimir 10 - -3\n";

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