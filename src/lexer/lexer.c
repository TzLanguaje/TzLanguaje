#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ==========================
 * UTILIDADES
 * ==========================
 */

static char *copy_string(
    const char *start,
    size_t length
) {
    char *result = malloc(length + 1);

    if (result == NULL) {
        return NULL;
    }

    memcpy(result, start, length);

    result[length] = '\0';

    return result;
}

/*
 * ==========================
 * PALABRAS RESERVADAS
 * ==========================
 */

static TokenType keyword_type(
    const char *value
) {
    if (strcmp(value, "variable") == 0) {
        return TOKEN_VARIABLE;
    }

    if (strcmp(value, "imprimir") == 0) {
        return TOKEN_IMPRIMIR;
    }

    if (strcmp(value, "verdadero") == 0) {
        return TOKEN_TRUE;
    }

    if (strcmp(value, "falso") == 0) {
        return TOKEN_FALSE;
    }

    return TOKEN_IDENTIFIER;
}

/*
 * ==========================
 * NOMBRE DEL TOKEN
 * ==========================
 */

const char *token_type_name(
    TokenType type
) {
    switch (type) {

        case TOKEN_EOF:
            return "EOF";

        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";

        case TOKEN_NUMBER:
            return "NUMBER";

        case TOKEN_DECIMAL:
            return "DECIMAL";

        case TOKEN_STRING:
            return "STRING";

        case TOKEN_VARIABLE:
            return "VARIABLE";

        case TOKEN_IMPRIMIR:
            return "IMPRIMIR";

        case TOKEN_TRUE:
            return "TRUE";

        case TOKEN_FALSE:
            return "FALSE";

        case TOKEN_PLUS:
            return "PLUS";

        case TOKEN_MINUS:
            return "MINUS";

        case TOKEN_STAR:
            return "STAR";

        case TOKEN_SLASH:
            return "SLASH";

        case TOKEN_EQUAL:
            return "EQUAL";

        case TOKEN_SEMICOLON:
            return "SEMICOLON";

        case TOKEN_LPAREN:
            return "LPAREN";

        case TOKEN_RPAREN:
            return "RPAREN";

        default:
            return "UNKNOWN";
    }
}

/*
 * ==========================
 * LEXER
 * ==========================
 */

Token *lexer_tokenize(
    const char *source,
    int *token_count
) {
    size_t capacity = 16;
    int count = 0;

    Token *tokens =
        malloc(sizeof(Token) * capacity);

    if (tokens == NULL) {
        return NULL;
    }

    const char *current = source;

    int line = 1;

    while (*current != '\0') {

        /*
         * ESPACIOS
         */

        if (isspace((unsigned char)*current)) {

            if (*current == '\n') {
                line++;
            }

            current++;

            continue;
        }

        /*
         * COMENTARIOS //
         */

        if (
            *current == '/' &&
            *(current + 1) == '/'
        ) {
            current += 2;

            while (
                *current != '\0' &&
                *current != '\n'
            ) {
                current++;
            }

            continue;
        }

        /*
         * CRECER ARRAY
         */

        if (count >= (int)capacity) {

            capacity *= 2;

            Token *new_tokens =
                realloc(
                    tokens,
                    sizeof(Token) * capacity
                );

            if (new_tokens == NULL) {

                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            tokens = new_tokens;
        }

        Token token;

        token.line = line;
        token.value = NULL;

        /*
         * ==========================
         * IDENTIFICADORES
         * ==========================
         */

        if (
            isalpha((unsigned char)*current) ||
            *current == '_'
        ) {
            const char *start = current;

            while (
                isalnum((unsigned char)*current) ||
                *current == '_'
            ) {
                current++;
            }

            size_t length =
                current - start;

            token.value =
                copy_string(
                    start,
                    length
                );

            if (token.value == NULL) {

                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            token.type =
                keyword_type(token.value);

            tokens[count++] = token;

            continue;
        }

        /*
         * ==========================
         * NÚMEROS
         * ==========================
         *
         * 10
         * 20
         * 3.14
         * 1.78
         */

        if (
            isdigit(
                (unsigned char)*current
            )
        ) {
            const char *start = current;

            while (
                isdigit(
                    (unsigned char)*current
                )
            ) {
                current++;
            }

            TokenType number_type =
                TOKEN_NUMBER;

            /*
             * Decimal
             */

            if (
                *current == '.' &&
                isdigit(
                    (unsigned char)*(current + 1)
                )
            ) {
                number_type =
                    TOKEN_DECIMAL;

                current++;

                while (
                    isdigit(
                        (unsigned char)*current
                    )
                ) {
                    current++;
                }
            }

            size_t length =
                current - start;

            token.type =
                number_type;

            token.value =
                copy_string(
                    start,
                    length
                );

            if (token.value == NULL) {

                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            tokens[count++] = token;

            continue;
        }

        /*
         * ==========================
         * STRINGS
         * ==========================
         */

        if (*current == '"') {

            current++;

            const char *start =
                current;

            while (
                *current != '\0' &&
                *current != '"'
            ) {
                current++;
            }

            size_t length =
                current - start;

            token.type =
                TOKEN_STRING;

            token.value =
                copy_string(
                    start,
                    length
                );

            if (token.value == NULL) {

                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            /*
             * Cerrar comillas
             */

            if (*current == '"') {
                current++;
            }

            tokens[count++] = token;

            continue;
        }

        /*
         * ==========================
         * OPERADORES
         * ==========================
         */

        switch (*current) {

            case '+':

                token.type =
                    TOKEN_PLUS;

                token.value =
                    copy_string("+", 1);

                current++;

                break;

            case '-':

                token.type =
                    TOKEN_MINUS;

                token.value =
                    copy_string("-", 1);

                current++;

                break;

            case '*':

                token.type =
                    TOKEN_STAR;

                token.value =
                    copy_string("*", 1);

                current++;

                break;

            case '/':

                token.type =
                    TOKEN_SLASH;

                token.value =
                    copy_string("/", 1);

                current++;

                break;

            case '=':

                token.type =
                    TOKEN_EQUAL;

                token.value =
                    copy_string("=", 1);

                current++;

                break;

            case ';':

                token.type =
                    TOKEN_SEMICOLON;

                token.value =
                    copy_string(";", 1);

                current++;

                break;

            case '(':

                token.type =
                    TOKEN_LPAREN;

                token.value =
                    copy_string("(", 1);

                current++;

                break;

            case ')':

                token.type =
                    TOKEN_RPAREN;

                token.value =
                    copy_string(")", 1);

                current++;

                break;

            default:

                fprintf(
                    stderr,
                    "Lexer error at line %d: unexpected character '%c'\n",
                    line,
                    *current
                );

                current++;

                continue;
        }

        if (token.value == NULL) {

            lexer_free_tokens(
                tokens,
                count
            );

            return NULL;
        }

        tokens[count++] = token;
    }

    /*
     * ==========================
     * EOF
     * ==========================
     */

    Token eof;

    eof.type =
        TOKEN_EOF;

    eof.value =
        copy_string("", 0);

    eof.line =
        line;

    if (eof.value == NULL) {

        lexer_free_tokens(
            tokens,
            count
        );

        return NULL;
    }

    tokens[count++] = eof;

    *token_count = count;

    return tokens;
}

/*
 * ==========================
 * LIBERAR TOKENS
 * ==========================
 */

void lexer_free_tokens(
    Token *tokens,
    int token_count
) {
    if (tokens == NULL) {
        return;
    }

    for (
        int i = 0;
        i < token_count;
        i++
    ) {
        free(tokens[i].value);
    }

    free(tokens);
}
