#ifndef TZLANG_PARSER_H
#define TZLANG_PARSER_H

#include "../ast/ast.h"
#include "../lexer/lexer.h"

typedef struct {
    Token *tokens;
    int token_count;
    int current;

    /*
     * >0 mientras se analiza el
     * cuerpo de una función.
     *
     * Sirve para rechazar
     * 'retornar' fuera de una
     * función en tiempo de
     * análisis, incluso si esa
     * línea nunca llegara a
     * ejecutarse.
     */
    int function_depth;

    /*
     * >0 mientras se analiza el
     * cuerpo de un 'mientras'.
     *
     * Permite rechazar 'romper' y
     * 'continuar' fuera de un bucle
     * durante el análisis, aunque
     * esa línea nunca se ejecutase.
     *
     * Se reinicia al entrar en el
     * cuerpo de una función: una
     * función declarada dentro de
     * un bucle no puede romperlo,
     * porque se la puede llamar
     * desde cualquier sitio.
     */
    int loop_depth;
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