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
     * Fin de instrucción
     *
     * TzLang usa el salto de línea
     * para terminar instrucciones.
     * El ';' sigue siendo válido.
     */

    TOKEN_NEWLINE,

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
    TOKEN_NULO,

    /*
     * Control de flujo
     *
     * si
     * sino
     * mientras
     * fin
     */

    TOKEN_SI,
    TOKEN_SINO,
    TOKEN_MIENTRAS,
    TOKEN_FIN,

    /*
     * Operadores lógicos
     *
     * y
     * o
     * no
     */

    TOKEN_Y,
    TOKEN_O,
    TOKEN_NO,

    /*
     * Comparaciones en pseudocódigo
     *
     * es mayor que
     * es menor que
     * es mayor o igual que
     * es menor o igual que
     * es igual a
     * es diferente de
     */

    TOKEN_ES,
    TOKEN_MAYOR,
    TOKEN_MENOR,
    TOKEN_IGUAL,
    TOKEN_DIFERENTE,
    TOKEN_QUE,
    TOKEN_A,
    TOKEN_DE,

    /*
     * Operadores
     */

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,

    /*
     * Comparación simbólica
     *
     * Se mantiene por
     * compatibilidad.
     */

    TOKEN_GREATER,
    TOKEN_LESS,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_NOT_EQUAL,

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
