#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"

int main(void) {

const char *source =
    "variable edad = 20\n"
    "variable documento = verdadero\n"
    "\n"
    "si (edad es mayor que 10)\n"
    "    imprimir \"Mayor que 10\"\n"
    "fin\n"
    "\n"
    "si (edad es mayor o igual que 18) y (documento)\n"
    "    imprimir \"Puede entrar\"\n"
    "fin\n"
    "\n"
    "si (edad es menor que 10) o (documento)\n"
    "    imprimir \"Segunda condición\"\n"
    "fin\n"
    "\n"
    "si (edad es igual a 20)\n"
    "    imprimir \"Tiene 20\"\n"
    "fin\n"
    "\n"
    "si (edad es diferente de 30)\n"
    "    imprimir \"No tiene 30\"\n"
    "fin\n"
    "\n"
    "si (edad es mayor o igual que 18) y (documento o falso)\n"
    "    imprimir \"Condición compleja\"\n"
    "fin\n";
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