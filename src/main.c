#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"

int main(void) {

    const char *source =
        "variable edad = 19;\n"
        "variable altura = 1.78;\n"
        "variable activo = verdadero;\n"
        "variable eliminado = falso;\n"
        "imprimir(edad);";

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
            "Error ejecutando Lexer.\n"
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
            "Error creando Parser.\n"
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
            "Error construyendo AST.\n"
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
     * AST
     * ==========================
     */

    printf(
        "=== TzLang AST ===\n\n"
    );

    ast_print_tree(
        program,
        0
    );

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