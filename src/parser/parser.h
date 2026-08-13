#ifndef TZLANG_PARSER_H
#define TZLANG_PARSER_H

#include "../ast/ast.h"
#include "../lexer/lexer.h"

typedef struct {
    Token *tokens;
    int token_count;
    int current;
} Parser;

/*
 * Crear un parser
 */
Parser *parser_create(
    Token *tokens,
    int token_count
);

/*
 * Liberar el parser
 */
void parser_free(Parser *parser);

/*
 * Analizar todos los tokens y construir el AST
 */
ASTNode *parser_parse(Parser *parser);

#endif