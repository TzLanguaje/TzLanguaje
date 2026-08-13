#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"

int main(void) {

const char *source =
    "// PRUEBA 1 - scope LEXICO: gana donde se DEFINIO\n"
    "variable x = \"global\"\n"
    "funcion interna()\n"
    "    imprimir x\n"
    "fin\n"
    "funcion externa()\n"
    "    variable x = \"externa\"\n"
    "    interna()\n"
    "fin\n"
    "externa()\n"
    "\n"
    "// PRUEBA 2 - la local tapa a la global\n"
    "variable saludo = \"global\"\n"
    "funcion prueba2()\n"
    "    variable saludo = \"local\"\n"
    "    imprimir saludo\n"
    "fin\n"
    "prueba2()\n"
    "imprimir saludo\n"
    "\n"
    "// PRUEBA 3 - asignar sube por la cadena lexica\n"
    "variable z = 10\n"
    "funcion prueba3()\n"
    "    z = 20\n"
    "fin\n"
    "prueba3()\n"
    "imprimir z\n"
    "\n"
    "// PRUEBA 4 - asignar a la local no toca la global\n"
    "variable w = 10\n"
    "funcion prueba4()\n"
    "    variable w = 20\n"
    "    w = 30\n"
    "    imprimir w\n"
    "fin\n"
    "prueba4()\n"
    "imprimir w\n";
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