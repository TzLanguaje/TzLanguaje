#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Verifica cuál es el token actual.
 */
static Token *current_token(Parser *parser) {
    if (
        parser == NULL ||
        parser->current >= parser->token_count
    ) {
        return NULL;
    }

    return &parser->tokens[parser->current];
}

/*
 * Avanza al siguiente token.
 */
static void advance(Parser *parser) {
    if (
        parser != NULL &&
        parser->current < parser->token_count
    ) {
        parser->current++;
    }
}

/*
 * Comprueba si el token actual
 * es del tipo indicado.
 */
static int check(
    Parser *parser,
    TokenType type
) {
    Token *token = current_token(parser);

    if (token == NULL) {
        return 0;
    }

    return token->type == type;
}

/*
 * Comprueba el token actual y,
 * si coincide, avanza.
 */
static int match(
    Parser *parser,
    TokenType type
) {
    if (!check(parser, type)) {
        return 0;
    }

    advance(parser);

    return 1;
}

/*
 * Crea un error del Parser.
 */
static void parser_error(
    Parser *parser,
    const char *message
) {
    Token *token = current_token(parser);

    if (token == NULL) {
        fprintf(
            stderr,
            "Error del Parser: %s\n",
            message
        );

        return;
    }

    fprintf(
        stderr,
        "Error del Parser en línea %d: %s\n",
        token->line,
        message
    );
}

/*
 * Crear Parser
 */
Parser *parser_create(
    Token *tokens,
    int token_count
) {
    Parser *parser = malloc(sizeof(Parser));

    if (parser == NULL) {
        return NULL;
    }

    parser->tokens = tokens;
    parser->token_count = token_count;
    parser->current = 0;

    return parser;
}

/*
 * Liberar Parser
 */
void parser_free(Parser *parser) {
    free(parser);
}

/*
 * NUMBER
 *
 * Ejemplo:
 *
 * 10
 */
static ASTNode *parse_primary(Parser *parser) {

    Token *token = current_token(parser);

    if (token == NULL) {
        parser_error(
            parser,
            "Se esperaba una expresión."
        );

        return NULL;
    }

    /*
     * Número
     */
    if (match(parser, TOKEN_NUMBER)) {

        int value = atoi(token->value);

        return ast_number(value);
    }

    /*
     * String
     */
    if (match(parser, TOKEN_STRING)) {

        return ast_string(token->value);
    }

    /*
     * Identificador
     *
     * Ejemplo:
     *
     * x
     */
    if (match(parser, TOKEN_IDENTIFIER)) {

        return ast_identifier(token->value);
    }

    parser_error(
        parser,
        "Se esperaba un número, texto o identificador."
    );

    return NULL;
}

/*
 * EXPRESIONES
 *
 * Por ahora soportamos:
 *
 * 10
 * x
 * 10 + 20
 * x + y
 *
 * Más adelante añadiremos:
 *
 * -
 * *
 * /
 * &&
 * ||
 * ==
 * >
 * <
 */
static ASTNode *parse_expression(Parser *parser) {

    ASTNode *left =
        parse_primary(parser);

    if (left == NULL) {
        return NULL;
    }

    while (
        check(parser, TOKEN_PLUS) ||
        check(parser, TOKEN_MINUS) ||
        check(parser, TOKEN_STAR) ||
        check(parser, TOKEN_SLASH)
    ) {

        Token *operator_token =
            current_token(parser);

        char operator =
            operator_token->value[0];

        advance(parser);

        ASTNode *right =
            parse_primary(parser);

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        left =
            ast_binary(
                left,
                operator,
                right
            );
    }

    return left;
}

/*
 * DECLARACIÓN DE VARIABLE
 *
 * variable x = 10;
 */
static ASTNode *parse_variable(
    Parser *parser
) {

    /*
     * Ya consumimos:
     *
     * variable
     *
     * Ahora esperamos:
     *
     * x
     */

    Token *name =
        current_token(parser);

    if (
        name == NULL ||
        name->type != TOKEN_IDENTIFIER
    ) {

        parser_error(
            parser,
            "Se esperaba el nombre de la variable."
        );

        return NULL;
    }

    advance(parser);

    /*
     * =
     */

    if (!match(parser, TOKEN_EQUAL)) {

        parser_error(
            parser,
            "Se esperaba '=' después del nombre de la variable."
        );

        return NULL;
    }

    /*
     * Valor
     */

    ASTNode *value =
        parse_expression(parser);

    if (value == NULL) {
        return NULL;
    }

    /*
     * ;
     */

    if (!match(parser, TOKEN_SEMICOLON)) {

        parser_error(
            parser,
            "Se esperaba ';' después de la declaración."
        );

        ast_free(value);

        return NULL;
    }

    return ast_variable(
        name->value,
        value
    );
}

/*
 * IMPRIMIR
 *
 * imprimir(x);
 */
static ASTNode *parse_print(
    Parser *parser
) {

    /*
     * Ya consumimos:
     *
     * imprimir
     *
     * Ahora esperamos:
     *
     * (
     */

    if (!match(parser, TOKEN_LPAREN)) {

        parser_error(
            parser,
            "Se esperaba '(' después de 'imprimir'."
        );

        return NULL;
    }

    /*
     * Expresión
     */

    ASTNode *expression =
        parse_expression(parser);

    if (expression == NULL) {
        return NULL;
    }

    /*
     * )
     */

    if (!match(parser, TOKEN_RPAREN)) {

        parser_error(
            parser,
            "Se esperaba ')' después de la expresión."
        );

        ast_free(expression);

        return NULL;
    }

    /*
     * ;
     */

    if (!match(parser, TOKEN_SEMICOLON)) {

        parser_error(
            parser,
            "Se esperaba ';' después de 'imprimir'."
        );

        ast_free(expression);

        return NULL;
    }

    return ast_print(expression);
}

/*
 * INSTRUCCIÓN
 */
static ASTNode *parse_statement(
    Parser *parser
) {

    /*
     * variable
     */
    if (match(parser, TOKEN_VARIABLE)) {

        return parse_variable(parser);
    }

    /*
     * imprimir
     */
    if (match(parser, TOKEN_IMPRIMIR)) {

        return parse_print(parser);
    }

    parser_error(
        parser,
        "Instrucción desconocida."
    );

    return NULL;
}

/*
 * PROGRAMA COMPLETO
 */
ASTNode *parser_parse(Parser *parser) {

    if (parser == NULL) {
        return NULL;
    }

    ASTNode *program =
        ast_program();

    if (program == NULL) {
        return NULL;
    }

    while (
        !check(parser, TOKEN_EOF)
    ) {

        ASTNode *statement =
            parse_statement(parser);

        if (statement == NULL) {

            ast_free(program);

            return NULL;
        }

        ast_program_add(
            program,
            statement
        );
    }

    return program;
}