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
     * Control del bucle
     *
     * romper
     * continuar
     */

    TOKEN_ROMPER,
    TOKEN_CONTINUAR,

    /*
     * Recorrido de listas
     *
     * para cada numero en numeros
     *     ...
     * fin
     */

    TOKEN_PARA,
    TOKEN_CADA,
    TOKEN_EN,

    /*
     * Funciones
     *
     * funcion nombre()
     *     ...
     * fin
     */

    TOKEN_FUNCION,
    TOKEN_RETORNAR,

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

    /*
     * Separa parámetros y
     * argumentos:
     *
     * sumar(a, b)
     */

    TOKEN_COMMA,

    TOKEN_LPAREN,
    TOKEN_RPAREN,

    /*
     * Listas e indexación
     *
     * [10, 20, 30]
     * numeros[0]
     */

    TOKEN_LBRACKET,
    TOKEN_RBRACKET,

    /*
     * Diccionarios
     *
     * {"nombre": "Carlos"}
     */

    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_COLON

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
 *
 * Devuelve NULL si la
 * tokenizacion FALLA, ya sea por
 * falta de memoria o porque el
 * archivo contiene caracteres que
 * TzLang no reconoce.
 *
 * En el caso de los caracteres
 * desconocidos se recorre el
 * archivo ENTERO y se informa de
 * todos antes de devolver NULL,
 * para no obligar a corregirlos
 * de uno en uno.
 *
 * Devolver NULL es lo que impide
 * que un programa con errores
 * lexicos llegue a ejecutarse:
 * quien llama ya trataba ese caso
 * como fallo.
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
