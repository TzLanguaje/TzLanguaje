#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "interpreter/interpreter.h"

int main(void) {

const char *source =
    "variable persona = {\"nombre\": \"Carlos\", \"edad\": 20}\n"
    "variable numeros = [1, 2, 3]\n"
    "\n"
    "// largo / tipo\n"
    "imprimir largo(\"hola\")\n"
    "imprimir largo(numeros)\n"
    "imprimir largo(persona)\n"
    "imprimir tipo(3.14)\n"
    "imprimir tipo(persona)\n"
    "\n"
    "// conversiones\n"
    "imprimir texto(20) + \" anos\"\n"
    "imprimir numero(\"42\") + 1\n"
    "imprimir decimal(\"3.5\") * 2\n"
    "\n"
    "// listas\n"
    "agregar(numeros, 4)\n"
    "imprimir numeros\n"
    "imprimir eliminar(numeros, 0)\n"
    "imprimir numeros\n"
    "imprimir contiene(numeros, 3)\n"
    "\n"
    "// diccionarios\n"
    "imprimir contiene(persona, \"edad\")\n"
    "imprimir eliminar(persona, \"edad\")\n"
    "imprimir persona\n"
    "\n"
    "// como expresiones y anidadas\n"
    "variable cantidad = largo([1, 2, 3])\n"
    "imprimir cantidad\n"
    "imprimir texto(largo(persona))\n";

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