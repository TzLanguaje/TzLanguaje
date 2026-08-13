#ifndef TZLANG_LEXER_H
#define TZLANG_LEXER_H

/*
 * ==========================
 * TIPOS DE TOKEN
 * ==========================
 */

typedef enum {

    TOKEN_EOF,

    /*
     * Valores
     */

    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_DECIMAL,
    TOKEN_STRING,

    /*
     * Palabras reservadas
     */

    TOKEN_VARIABLE,
    TOKEN_IMPRIMIR,

    TOKEN_TRUE,
    TOKEN_FALSE,

    /*
     * Operadores
     */

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,

    /*
     * Símbolos
     */

    TOKEN_EQUAL,
    TOKEN_SEMICOLON,

    TOKEN_LPAREN,
    TOKEN_RPAREN

} TokenType;


/*
 * ==========================
 * TOKEN
 * ==========================
 */

typedef struct {

    TokenType type;

    char *value;

    int line;

} Token;


/*
 * ==========================
 * LEXER
 * ==========================
 */

/*
 * Convierte código TzLang
 * en una lista de tokens.
 */
Token *lexer_tokenize(
    const char *source,
    int *token_count
);


/*
 * Libera los tokens.
 */
void lexer_free_tokens(
    Token *tokens,
    int token_count
);


/*
 * Devuelve el nombre del token.
 *
 * Ejemplo:
 *
 * TOKEN_NUMBER
 *      ↓
 * "NUMBER"
 */
const char *token_type_name(
    TokenType type
);

#endif