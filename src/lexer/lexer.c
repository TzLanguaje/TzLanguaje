#include "lexer.h"

#include "../diagnostic/diagnostic.h"

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

/*
 * Tabla de palabras reservadas.
 *
 * Todo lo que no esté aquí
 * es un IDENTIFIER.
 */

typedef struct {
    const char *word;
    TokenType type;
} Keyword;

static const Keyword keywords[] = {

    /*
     * Declaraciones
     */

    { "variable",  TOKEN_VARIABLE  },
    { "imprimir",  TOKEN_IMPRIMIR  },

    /*
     * Literales
     */

    { "verdadero", TOKEN_TRUE      },
    { "falso",     TOKEN_FALSE     },
    { "nulo",      TOKEN_NULO      },

    /*
     * Control de flujo
     */

    { "si",        TOKEN_SI        },
    { "sino",      TOKEN_SINO      },
    { "mientras",  TOKEN_MIENTRAS  },
    { "fin",       TOKEN_FIN       },
    { "romper",    TOKEN_ROMPER    },
    { "continuar", TOKEN_CONTINUAR },

    /*
     * para cada X en LISTA
     *
     * 'cada' y 'en' solo son
     * especiales dentro de esa
     * cabecera, así que el parser
     * las acepta como nombres
     * normales fuera de ella.
     */

    { "para",      TOKEN_PARA      },
    { "cada",      TOKEN_CADA      },
    { "en",        TOKEN_EN        },

    /*
     * Funciones
     */

    { "funcion",   TOKEN_FUNCION   },
    { "retornar",  TOKEN_RETORNAR  },

    /*
     * Lógicos
     */

    { "y",         TOKEN_Y         },
    { "o",         TOKEN_O         },
    { "no",        TOKEN_NO        },

    /*
     * Comparaciones en pseudocódigo
     *
     * es mayor que
     * es menor o igual que
     * es igual a
     * es diferente de
     */

    { "es",        TOKEN_ES        },
    { "mayor",     TOKEN_MAYOR     },
    { "menor",     TOKEN_MENOR     },
    { "igual",     TOKEN_IGUAL     },
    { "diferente", TOKEN_DIFERENTE },
    { "que",       TOKEN_QUE       },
    { "a",         TOKEN_A         },
    { "de",        TOKEN_DE        }
};

static TokenType keyword_type(
    const char *value
) {
    size_t count =
        sizeof(keywords) /
        sizeof(keywords[0]);

    for (size_t i = 0; i < count; i++) {

        if (
            strcmp(
                value,
                keywords[i].word
            ) == 0
        ) {

            return keywords[i].type;
        }
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

        case TOKEN_NEWLINE:
            return "NEWLINE";

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

        case TOKEN_NULO:
            return "NULO";

        case TOKEN_SI:
            return "SI";

        case TOKEN_SINO:
            return "SINO";

        case TOKEN_MIENTRAS:
            return "MIENTRAS";

        case TOKEN_FIN:
            return "FIN";

        case TOKEN_ROMPER:
            return "ROMPER";

        case TOKEN_CONTINUAR:
            return "CONTINUAR";

        case TOKEN_PARA:
            return "PARA";

        case TOKEN_CADA:
            return "CADA";

        case TOKEN_EN:
            return "EN";

        case TOKEN_FUNCION:
            return "FUNCION";

        case TOKEN_RETORNAR:
            return "RETORNAR";

        case TOKEN_Y:
            return "Y";

        case TOKEN_O:
            return "O";

        case TOKEN_NO:
            return "NO";

        case TOKEN_ES:
            return "ES";

        case TOKEN_MAYOR:
            return "MAYOR";

        case TOKEN_MENOR:
            return "MENOR";

        case TOKEN_IGUAL:
            return "IGUAL";

        case TOKEN_DIFERENTE:
            return "DIFERENTE";

        case TOKEN_QUE:
            return "QUE";

        case TOKEN_A:
            return "A";

        case TOKEN_DE:
            return "DE";

        case TOKEN_PLUS:
            return "PLUS";

        case TOKEN_MINUS:
            return "MINUS";

        case TOKEN_STAR:
            return "STAR";

        case TOKEN_SLASH:
            return "SLASH";

        case TOKEN_GREATER:
            return "GREATER";

        case TOKEN_LESS:
            return "LESS";

        case TOKEN_GREATER_EQUAL:
            return "GREATER_EQUAL";

        case TOKEN_LESS_EQUAL:
            return "LESS_EQUAL";

        case TOKEN_EQUAL_EQUAL:
            return "EQUAL_EQUAL";

        case TOKEN_NOT_EQUAL:
            return "NOT_EQUAL";

        case TOKEN_EQUAL:
            return "EQUAL";

        case TOKEN_SEMICOLON:
            return "SEMICOLON";

        case TOKEN_COMMA:
            return "COMMA";

        case TOKEN_LPAREN:
            return "LPAREN";

        case TOKEN_RPAREN:
            return "RPAREN";

        case TOKEN_LBRACKET:
            return "LBRACKET";

        case TOKEN_RBRACKET:
            return "RBRACKET";

        case TOKEN_LBRACE:
            return "LBRACE";

        case TOKEN_RBRACE:
            return "RBRACE";

        case TOKEN_COLON:
            return "COLON";

        default:
            return "UNKNOWN";
    }
}

/*
 * ==========================
 * LEXER
 * ==========================
 */

/*
 * ==========================
 * PISTA SEGUN EL CARACTER
 * ==========================
 *
 * Casi siempre que aparece un caracter
 * que TzLang no conoce, la persona
 * venia de otro lenguaje y escribio la
 * version de alli.
 *
 * Decir "caracter inesperado" es
 * cierto y no ayuda. Decir "para negar
 * se usa 'no'" enseña algo.
 *
 * Devuelve NULL si no hay nada util
 * que anadir.
 */

static const char *pista_para(char c) {

    switch (c) {

        case '\'':
            return "Los textos van entre comillas dobles: \"así\".";

        case '.':
            return "Para leer un diccionario se usan corchetes: persona[\"nombre\"].";

        case '#':
            return "Los comentarios empiezan por dos barras: // así.";

        case '&':
            return "Para unir dos condiciones se escribe 'y'.";

        case '|':
            return "Para elegir entre dos condiciones se escribe 'o'.";

        case '!':
            return "Para negar una condición se escribe 'no'.";

        case '$':
        case '@':
            return "Las variables se nombran sin símbolos delante: variable edad = 20.";

        case '^':
            return "No hay operador de potencia: multiplica las veces que haga falta.";

        default:
            return NULL;
    }
}

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

    /*
     * Se sigue leyendo despues de un
     * caracter desconocido para poder
     * informar de todos, pero al final
     * la tokenizacion se considera
     * fallida.
     */

    int had_error = 0;

    while (*current != '\0') {

        /*
         * ESPACIOS
         *
         * El salto de línea NO se
         * ignora: termina la
         * instrucción, así que se
         * convierte en un token.
         */

        if (
            *current != '\n' &&
            isspace((unsigned char)*current)
        ) {
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
         * SALTO DE LÍNEA
         * ==========================
         *
         * Marca el final de una
         * instrucción:
         *
         * variable edad = 20
         * imprimir edad
         */

        if (*current == '\n') {

            token.type =
                TOKEN_NEWLINE;

            token.value =
                copy_string("\n", 1);

            if (token.value == NULL) {

                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            current++;

            line++;

            tokens[count++] = token;

            continue;
        }

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

            /*
             * ==========================
             * SECUENCIAS DE ESCAPE
             * ==========================
             *
             * El texto se construye
             * caracter a caracter en vez
             * de copiarse de golpe,
             * porque hay que traducir:
             *
             *   \n   salto de linea
             *   \t   tabulador
             *   \"   comilla doble
             *   \\   barra invertida
             *
             * Sin esto no habia forma de
             * meter una comilla dentro
             * de un texto.
             *
             * El buffer se dimensiona
             * con lo que queda de
             * archivo: traducir siempre
             * acorta, nunca alarga.
             */

            size_t restante = strlen(current);

            char *texto = malloc(restante + 1);

            if (texto == NULL) {

                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            size_t escritos = 0;
            int escape_malo = 0;
            char cual_malo = 0;

            while (
                *current != '\0' &&
                *current != '"'
            ) {

                if (*current != '\\') {

                    texto[escritos++] = *current;

                    current++;

                    continue;
                }

                /*
                 * Una barra al final del
                 * archivo no escapa nada.
                 */

                if (*(current + 1) == '\0') {
                    break;
                }

                switch (*(current + 1)) {

                    case 'n':
                        texto[escritos++] = '\n';
                        break;

                    case 't':
                        texto[escritos++] = '\t';
                        break;

                    case '"':
                        texto[escritos++] = '"';
                        break;

                    case '\\':
                        texto[escritos++] = '\\';
                        break;

                    default:
                        escape_malo = 1;
                        cual_malo = *(current + 1);
                        break;
                }

                if (escape_malo) {
                    break;
                }

                current += 2;
            }

            texto[escritos] = '\0';

            if (escape_malo) {

                diagnostic_registrar(DIAG_LEXICO);

                fprintf(
                    stderr,
                    "Error en línea %d: '\\%c' no es una secuencia valida. "
                    "Las validas son \\n, \\t, \\\" y \\\\.\n",
                    token.line,
                    cual_malo
                );

                free(texto);

                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            (void) start;

            token.type =
                TOKEN_STRING;

            token.value =
                copy_string(
                    texto,
                    escritos
                );

            free(texto);

            if (token.value == NULL) {

                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            /*
             * Cerrar comillas
             *
             * Si se llego al final del
             * archivo sin encontrar la
             * comilla de cierre, el
             * texto esta sin terminar.
             *
             * Antes esto pasaba en
             * silencio: el programa se
             * ejecutaba y salia con
             * codigo 0, como si nada.
             * Olvidar una comilla es
             * de los descuidos mas
             * comunes al empezar, y
             * merece un error, no un
             * exito enganoso.
             */

            if (*current != '"') {

                diagnostic_registrar(DIAG_TEXTO_SIN_CERRAR);

                fprintf(
                    stderr,
                    "Error en línea %d: falta la comilla que cierra el texto.\n",
                    token.line
                );

                free(token.value);

                lexer_free_tokens(
                    tokens,
                    count
                );

                return NULL;
            }

            current++;

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

            case '%':

                token.type =
                    TOKEN_PERCENT;

                token.value =
                    copy_string("%", 1);

                current++;

                break;

            case '/':

                token.type =
                    TOKEN_SLASH;

                token.value =
                    copy_string("/", 1);

                current++;

                break;

            /*
             * ==========================
             * COMPARACIÓN
             * ==========================
             *
             * >
             * <
             */

            case '>':

                if (*(current + 1) == '=') {

                    token.type =
                        TOKEN_GREATER_EQUAL;

                    token.value =
                        copy_string(">=", 2);

                    current += 2;

                    break;
                }

                token.type =
                    TOKEN_GREATER;

                token.value =
                    copy_string(">", 1);

                current++;

                break;

            case '<':

                if (*(current + 1) == '=') {

                    token.type =
                        TOKEN_LESS_EQUAL;

                    token.value =
                        copy_string("<=", 2);

                    current += 2;

                    break;
                }

                token.type =
                    TOKEN_LESS;

                token.value =
                    copy_string("<", 1);

                current++;

                break;

            /*
             * ==========================
             * = o ==
             * ==========================
             *
             * =  → asignación
             * == → comparación
             */

            case '=':

                if (*(current + 1) == '=') {

                    token.type =
                        TOKEN_EQUAL_EQUAL;

                    token.value =
                        copy_string("==", 2);

                    current += 2;

                    break;
                }

                token.type =
                    TOKEN_EQUAL;

                token.value =
                    copy_string("=", 1);

                current++;

                break;

            /*
             * ==========================
             * !=
             * ==========================
             *
             * '!' por sí solo todavía
             * no es un operador de TzLang.
             */

            case '!':

                if (*(current + 1) == '=') {

                    token.type =
                        TOKEN_NOT_EQUAL;

                    token.value =
                        copy_string("!=", 2);

                    current += 2;

                    break;
                }

                diagnostic_registrar(DIAG_LEXICO);

                {
                    const char *pista = pista_para(*current);

                    fprintf(
                        stderr,
                        "Error en línea %d: el carácter '%c' no forma parte de TzLang.\n",
                        line,
                        *current
                    );

                    if (pista != NULL) {
                        fprintf(stderr, "%s\n", pista);
                    }
                }

                had_error = 1;

                current++;

                continue;

            case ';':

                token.type =
                    TOKEN_SEMICOLON;

                token.value =
                    copy_string(";", 1);

                current++;

                break;

            case ',':

                token.type =
                    TOKEN_COMMA;

                token.value =
                    copy_string(",", 1);

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

            case '[':

                token.type =
                    TOKEN_LBRACKET;

                token.value =
                    copy_string("[", 1);

                current++;

                break;

            case ']':

                token.type =
                    TOKEN_RBRACKET;

                token.value =
                    copy_string("]", 1);

                current++;

                break;

            case '{':

                token.type =
                    TOKEN_LBRACE;

                token.value =
                    copy_string("{", 1);

                current++;

                break;

            case '}':

                token.type =
                    TOKEN_RBRACE;

                token.value =
                    copy_string("}", 1);

                current++;

                break;

            case ':':

                token.type =
                    TOKEN_COLON;

                token.value =
                    copy_string(":", 1);

                current++;

                break;

            default:

                diagnostic_registrar(DIAG_LEXICO);

                {
                    const char *pista = pista_para(*current);

                    fprintf(
                        stderr,
                        "Error en línea %d: el carácter '%c' no forma parte de TzLang.\n",
                        line,
                        *current
                    );

                    if (pista != NULL) {
                        fprintf(stderr, "%s\n", pista);
                    }
                }

                had_error = 1;

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

    /*
     * El bucle solo crece el array
     * antes de leer un token nuevo,
     * así que aquí puede quedar lleno.
     */

    if (count >= (int)capacity) {

        capacity += 1;

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

    /*
     * Hubo caracteres desconocidos:
     * ya se informo de todos, y ahora
     * se falla para que el programa
     * NO se ejecute.
     */

    if (had_error) {

        lexer_free_tokens(tokens, count);

        return NULL;
    }

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

const char *lexer_palabra_reservada(TokenType tipo) {

    size_t i;

    for (
        i = 0;
        i < sizeof(keywords) / sizeof(keywords[0]);
        i++
    ) {
        if (keywords[i].type == tipo) {
            return keywords[i].word;
        }
    }

    return NULL;
}
