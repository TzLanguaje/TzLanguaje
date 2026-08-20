#include "parser.h"

#include "../diagnostic/diagnostic.h"

#include <errno.h>
#include <limits.h>
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
 * Mira un token más adelante
 * sin consumirlo.
 *
 * Sirve para distinguir:
 *
 * edad = 25     (asignación)
 * edad + 1      (expresión)
 */
static Token *peek_token(
    Parser *parser,
    int offset
) {
    if (parser == NULL) {
        return NULL;
    }

    int index =
        parser->current + offset;

    if (
        index < 0 ||
        index >= parser->token_count
    ) {
        return NULL;
    }

    return &parser->tokens[index];
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
/*
 * ==========================
 * A QUE LINEA CULPAR
 * ==========================
 *
 * El parser se queja mirando el token
 * que tiene delante. Cuando el error
 * es "falta algo" -un parentesis, un
 * 'fin'- ese token suele ser el fin de
 * linea o el fin de archivo, que ya
 * estan en la linea SIGUIENTE.
 *
 * Resultado: el alumno miraba una
 * linea vacia. Y como todos los
 * archivos terminan con un salto,
 * pasaba practicamente siempre.
 *
 * Se retrocede hasta el ultimo token
 * con contenido real, que es donde la
 * persona escribio de verdad.
 */

static int linea_del_error(Parser *parser) {

    int i = parser->current;

    if (i >= parser->token_count) {
        i = parser->token_count - 1;
    }

    while (i > 0) {

        TokenType t = parser->tokens[i].type;

        if (t != TOKEN_EOF && t != TOKEN_NEWLINE) {
            break;
        }

        i--;
    }

    return parser->tokens[i].line;
}

static void parser_error(
    Parser *parser,
    const char *message
) {
    Token *token = current_token(parser);

    diagnostic_registrar(
        diagnostic_categoria_de_mensaje(message)
    );

    if (token == NULL) {
        fprintf(
            stderr,
            "Error del Parser: %s\n",
            message
        );

        return;
    }

    (void) token;

    fprintf(
        stderr,
        "Error del Parser en línea %d: %s\n",
        linea_del_error(parser),
        message
    );
}

/*
 * ==========================
 * FIN DE INSTRUCCIÓN
 * ==========================
 *
 * TzLang termina las instrucciones
 * con un salto de línea.
 *
 * El ';' se sigue aceptando por
 * compatibilidad.
 */

/*
 * Salta SOLO saltos de linea.
 *
 * Se usa dentro de delimitadores
 * -- [ ] { } ( ) -- donde el salto
 * de linea NO termina nada, para
 * poder escribir literales
 * multilinea:
 *
 * variable persona = {
 *     "nombre": "Carlos",
 *     "edad": 20
 * }
 *
 * Fuera de ahi el salto de linea
 * sigue terminando la instruccion.
 */

static void skip_newlines(Parser *parser) {

    while (check(parser, TOKEN_NEWLINE)) {
        advance(parser);
    }
}

static void skip_terminators(Parser *parser) {

    while (
        check(parser, TOKEN_NEWLINE) ||
        check(parser, TOKEN_SEMICOLON)
    ) {
        advance(parser);
    }
}

/*
 * ¿Estamos al final de una
 * instrucción?
 *
 * Vale tanto un terminador real
 * como 'fin', 'sino' o el final
 * del archivo.
 */
static int at_statement_end(Parser *parser) {

    return
        check(parser, TOKEN_NEWLINE) ||
        check(parser, TOKEN_SEMICOLON) ||
        check(parser, TOKEN_EOF) ||
        check(parser, TOKEN_FIN) ||
        check(parser, TOKEN_SINO) ||
        current_token(parser) == NULL;
}

/*
 * Consume el final de una instrucción.
 *
 * 'fin', 'sino' y el final del
 * archivo también cierran una
 * instrucción sin consumirse.
 */
static int expect_terminator(Parser *parser) {

    if (
        check(parser, TOKEN_NEWLINE) ||
        check(parser, TOKEN_SEMICOLON)
    ) {
        skip_terminators(parser);

        return 1;
    }

    if (at_statement_end(parser)) {
        return 1;
    }

    parser_error(
        parser,
        "Se esperaba un salto de línea al final de la instrucción."
    );

    return 0;
}

/*
 * ==========================
 * PALABRAS CONTEXTUALES
 * ==========================
 *
 * Estas palabras solo son
 * especiales DENTRO de una
 * comparación:
 *
 * es mayor o igual que
 * es diferente de
 *
 * Fuera de ahí son nombres
 * normales, así que esto
 * sigue siendo válido:
 *
 * variable mayor = 10
 * variable a = 5
 *
 * El Lexer las distingue igual;
 * es el Parser quien decide.
 */

static int is_contextual_name(
    TokenType type
) {

    return
        type == TOKEN_MAYOR ||
        type == TOKEN_MENOR ||
        type == TOKEN_IGUAL ||
        type == TOKEN_DIFERENTE ||
        type == TOKEN_QUE ||
        type == TOKEN_A ||
        type == TOKEN_DE ||

        /*
         * 'cada' y 'en' solo son
         * especiales en la cabecera
         * del 'para cada', así que
         * fuera de ella siguen
         * siendo nombres válidos:
         *
         * variable en = 5
         */

        type == TOKEN_CADA ||
        type == TOKEN_EN;
}

/*
 * ==========================
 * CONSTRUCTORES SEGUROS
 * ==========================
 *
 * Si falta memoria liberamos
 * los hijos para no perderlos.
 */

static ASTNode *make_binary(
    ASTNode *left,
    BinaryOperator operator,
    ASTNode *right
) {
    ASTNode *node =
        ast_binary(left, operator, right);

    if (node == NULL) {
        ast_free(left);
        ast_free(right);
    }

    return node;
}

static ASTNode *make_unary(
    UnaryOperator operator,
    ASTNode *operand
) {
    ASTNode *node =
        ast_unary(operator, operand);

    if (node == NULL) {
        ast_free(operand);
    }

    return node;
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
    parser->function_depth = 0;
    parser->loop_depth = 0;

    return parser;
}

/*
 * Liberar Parser
 */
void parser_free(Parser *parser) {
    free(parser);
}

/*
 * ==========================
 * PRECEDENCIA
 * ==========================
 *
 * De MAYOR a MENOR prioridad:
 *
 * 1. ( )
 * 2. no
 * 3. * /
 * 4. + -
 * 5. comparaciones
 * 6. y
 * 7. o
 *
 * Cada nivel es una función que
 * llama al nivel más prioritario.
 */

/*
 * ==========================
 * LITERALES NUMERICOS
 * ==========================
 *
 * Error situado en un token
 * concreto, no en el actual.
 */

static void parser_error_token(
    const Token *token,
    const char *message
) {

    diagnostic_registrar(
        diagnostic_categoria_de_mensaje(message)
    );

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
 * Convierte el texto de un literal
 * entero SIN depender de overflow.
 *
 * El lexer solo produce digitos, de
 * modo que la magnitud se lee con
 * strtoull -- garantizado de 64 bits
 * como minimo -- y despues se
 * comprueba contra el rango de int.
 * Nunca se convierte a int un valor
 * que no quepa.
 *
 * 'negative' viene del menos unario
 * pegado al literal, y es lo que
 * permite que -2147483648 sea valido
 * mientras 2147483648 no lo es.
 */

static ASTNode *make_number_literal(
    const Token *token,
    int negative
) {

    const char *text = token->value;

    errno = 0;

    char *end = NULL;

    unsigned long long magnitude =
        strtoull(text, &end, 10);

    if (end == text || *end != '\0') {

        parser_error_token(
            token,
            "número entero mal formado."
        );

        return NULL;
    }

    unsigned long long limit =
        negative
            ? (unsigned long long)INT_MAX + 1ULL
            : (unsigned long long)INT_MAX;

    if (errno == ERANGE || magnitude > limit) {

        char message[160];

        snprintf(
            message,
            sizeof(message),
            "el número %s%s está fuera del rango permitido "
            "(de -2147483648 a 2147483647).",
            negative ? "-" : "",
            text
        );

        parser_error_token(token, message);

        return NULL;
    }

    /*
     * -(long long)2147483648 cabe de
     * sobra en long long, asi que
     * llegar a INT_MIN es seguro.
     */

    int value =
        negative
            ? (int)(-(long long)magnitude)
            : (int)magnitude;

    return ast_number(value);
}

/*
 * Literal decimal.
 *
 * strtod en vez de atof para poder
 * detectar un valor que no quepa en
 * un double.
 */

static ASTNode *make_decimal_literal(
    const Token *token
) {

    const char *text = token->value;

    errno = 0;

    char *end = NULL;

    double value = strtod(text, &end);

    if (end == text || *end != '\0') {

        parser_error_token(
            token,
            "número decimal mal formado."
        );

        return NULL;
    }

    if (errno == ERANGE) {

        parser_error_token(
            token,
            "el número decimal está fuera del rango representable."
        );

        return NULL;
    }

    return ast_decimal(value);
}

static ASTNode *parse_expression(Parser *parser);
static ASTNode *parse_expression_interna(Parser *parser);

/*
 * Una llamada también es una
 * expresión primaria:
 *
 * imprimir sumar(10, 20) * 2
 *
 * Se define más abajo, junto al
 * resto de lo relativo a funciones.
 */
static ASTNode *parse_call_expression(Parser *parser);

/*
 * Indexación:
 *
 * numeros[0]
 * matriz[1][0]
 *
 * Se define junto al resto de la
 * cadena de precedencia.
 */
static ASTNode *parse_postfix(Parser *parser);

/*
 * Libera una lista de expresiones
 * a medio construir.
 *
 * La comparten los argumentos de
 * una llamada y los elementos de
 * un literal de lista.
 */
static void free_argument_list(
    ASTNode **arguments,
    int count
);

/*
 * NIVEL 1
 *
 * Valores y paréntesis.
 *
 * 19
 * 1.78
 * "TzLang"
 * verdadero
 * nulo
 * edad
 * (10 + 2)
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
     * ==========================
     * PARÉNTESIS
     * ==========================
     *
     * Agrupan y cambian la
     * precedencia:
     *
     * (10 + 2) * 3
     */

    if (match(parser, TOKEN_LPAREN)) {

        skip_newlines(parser);

        ASTNode *expression =
            parse_expression(parser);

        if (expression == NULL) {
            return NULL;
        }

        skip_newlines(parser);

        if (!match(parser, TOKEN_RPAREN)) {

            parser_error(
                parser,
                "Se esperaba ')' para cerrar la expresión."
            );

            ast_free(expression);

            return NULL;
        }

        return expression;
    }

    /*
     * ==========================
     * LITERAL DE DICCIONARIO
     * ==========================
     *
     * {}
     * {"nombre": "Carlos"}
     * {"a": 1, "b": {"c": 2}}
     *
     * Claves y valores usan
     * parse_expression, asi que el
     * anidamiento con listas y con
     * otros diccionarios sale solo.
     *
     * La clave debe producir un
     * texto al evaluarse; eso se
     * comprueba en ejecucion.
     */

    if (check(parser, TOKEN_LBRACE)) {

        advance(parser);

        skip_newlines(parser);

        ASTNode **keys = NULL;
        ASTNode **values = NULL;
        int count = 0;
        int capacity = 0;

        /*
         * Diccionario vacio
         */

        if (match(parser, TOKEN_RBRACE)) {

            return ast_dictionary(NULL, NULL, 0);
        }

        for (;;) {

            ASTNode *key =
                parse_expression(parser);

            if (key == NULL) {

                free_argument_list(keys, count);
                free_argument_list(values, count);

                return NULL;
            }

            skip_newlines(parser);

            if (!match(parser, TOKEN_COLON)) {

                parser_error(
                    parser,
                    "Se esperaba ':' entre la clave y el valor."
                );

                ast_free(key);

                free_argument_list(keys, count);
                free_argument_list(values, count);

                return NULL;
            }

            skip_newlines(parser);

            ASTNode *value =
                parse_expression(parser);

            if (value == NULL) {

                ast_free(key);

                free_argument_list(keys, count);
                free_argument_list(values, count);

                return NULL;
            }

            if (count >= capacity) {

                int new_capacity =
                    capacity == 0
                        ? 4
                        : capacity * 2;

                ASTNode **grown_keys =
                    realloc(
                        keys,
                        sizeof(ASTNode *) *
                        new_capacity
                    );

                if (grown_keys != NULL) {
                    keys = grown_keys;
                }

                ASTNode **grown_values =
                    grown_keys == NULL
                        ? NULL
                        : realloc(
                              values,
                              sizeof(ASTNode *) *
                              new_capacity
                          );

                if (
                    grown_keys == NULL ||
                    grown_values == NULL
                ) {

                    ast_free(key);
                    ast_free(value);

                    free_argument_list(keys, count);
                    free_argument_list(values, count);

                    return NULL;
                }

                values = grown_values;
                capacity = new_capacity;
            }

            keys[count] = key;
            values[count] = value;

            count++;

            skip_newlines(parser);

            /*
             * Una coma obliga a que
             * venga otro par, asi que
             * {"a":1,} y {,"a":1}
             * fallan solos.
             */

            if (match(parser, TOKEN_COMMA)) {

                skip_newlines(parser);

                continue;
            }

            break;
        }

        if (!match(parser, TOKEN_RBRACE)) {

            parser_error(
                parser,
                "Se esperaba ',' o '}' en el diccionario."
            );

            free_argument_list(keys, count);
            free_argument_list(values, count);

            return NULL;
        }

        ASTNode *node =
            ast_dictionary(keys, values, count);

        if (node == NULL) {

            free_argument_list(keys, count);
            free_argument_list(values, count);
        }

        return node;
    }

    /*
     * ==========================
     * LITERAL DE LISTA
     * ==========================
     *
     * []
     * [10, 20, 30]
     * [1 + 2, 3 * 4]
     * [[1, 2], [3, 4]]
     *
     * Los elementos usan
     * parse_expression, así que no
     * hace falta lógica propia de
     * operadores ni de anidamiento.
     */

    if (check(parser, TOKEN_LBRACKET)) {

        advance(parser);

        skip_newlines(parser);

        ASTNode **elements = NULL;
        int count = 0;
        int capacity = 0;

        /*
         * Lista vacía
         */

        if (match(parser, TOKEN_RBRACKET)) {

            return ast_list(NULL, 0);
        }

        for (;;) {

            ASTNode *element =
                parse_expression(parser);

            if (element == NULL) {

                free_argument_list(
                    elements,
                    count
                );

                return NULL;
            }

            if (count >= capacity) {

                int new_capacity =
                    capacity == 0
                        ? 4
                        : capacity * 2;

                ASTNode **grown =
                    realloc(
                        elements,
                        sizeof(ASTNode *) *
                        new_capacity
                    );

                if (grown == NULL) {

                    ast_free(element);

                    free_argument_list(
                        elements,
                        count
                    );

                    return NULL;
                }

                elements = grown;
                capacity = new_capacity;
            }

            elements[count] = element;

            count++;

            /*
             * Una coma obliga a que
             * venga otro elemento, así
             * que [1,] y [1,,2] fallan
             * solos al intentar leer
             * la expresión siguiente.
             */

            skip_newlines(parser);

            if (match(parser, TOKEN_COMMA)) {

                skip_newlines(parser);

                continue;
            }

            break;
        }

        if (!match(parser, TOKEN_RBRACKET)) {

            parser_error(
                parser,
                "Se esperaba ',' o ']' en la lista."
            );

            free_argument_list(
                elements,
                count
            );

            return NULL;
        }

        ASTNode *node =
            ast_list(elements, count);

        if (node == NULL) {

            free_argument_list(
                elements,
                count
            );
        }

        return node;
    }

    /*
     * ==========================
     * NUMBER
     * ==========================
     *
     * 19
     */

    if (match(parser, TOKEN_NUMBER)) {

        return make_number_literal(token, 0);
    }

    /*
     * ==========================
     * DECIMAL
     * ==========================
     *
     * 1.78
     */

    if (match(parser, TOKEN_DECIMAL)) {

        return make_decimal_literal(token);
    }

    /*
     * ==========================
     * STRING
     * ==========================
     *
     * "TzLang"
     */

    if (match(parser, TOKEN_STRING)) {

        return ast_string(
            token->value
        );
    }

    /*
     * ==========================
     * BOOLEAN TRUE
     * ==========================
     *
     * verdadero
     */

    if (match(parser, TOKEN_TRUE)) {

        return ast_boolean(1);
    }

    /*
     * ==========================
     * BOOLEAN FALSE
     * ==========================
     *
     * falso
     */

    if (match(parser, TOKEN_FALSE)) {

        return ast_boolean(0);
    }

    /*
     * ==========================
     * NULO
     * ==========================
     */

    if (match(parser, TOKEN_NULO)) {

        return ast_null();
    }

    /*
     * ==========================
     * LLAMADA A FUNCIÓN
     * ==========================
     *
     * sumar(10, 20)
     *
     * Un nombre seguido de '(' es
     * una llamada, y produce un
     * valor como cualquier otra
     * expresión primaria.
     */

    if (
        check(parser, TOKEN_IDENTIFIER) ||
        is_contextual_name(token->type)
    ) {

        Token *next =
            peek_token(parser, 1);

        if (
            next != NULL &&
            next->type == TOKEN_LPAREN
        ) {

            return parse_call_expression(parser);
        }
    }

    /*
     * ==========================
     * IDENTIFIER
     * ==========================
     *
     * x
     * nombre
     * edad
     *
     * También las palabras
     * contextuales:
     *
     * mayor
     * menor
     * a
     */

    if (
        check(parser, TOKEN_IDENTIFIER) ||
        is_contextual_name(token->type)
    ) {

        advance(parser);

        return ast_identifier(
            token->value
        );
    }

    parser_error(
        parser,
        "Se esperaba un número, decimal, texto, booleano o identificador."
    );

    return NULL;
}

/*
 * NIVEL 1.5
 *
 * Indexación
 *
 * numeros[0]
 * matriz[1][0]
 *
 * Es el nivel MÁS prioritario
 * después de los valores, por
 * encima de 'no':
 *
 * no lista[0]
 *      ↓
 * no (lista[0])
 *
 * El bucle es lo que permite
 * encadenar sin ningún caso
 * especial para matrices: cada
 * '[' envuelve lo anterior en un
 * AST_INDEX más.
 */
static ASTNode *parse_postfix(Parser *parser) {

    ASTNode *node =
        parse_primary(parser);

    if (node == NULL) {
        return NULL;
    }

    while (check(parser, TOKEN_LBRACKET)) {

        advance(parser);

        skip_newlines(parser);

        ASTNode *index =
            parse_expression(parser);

        if (index == NULL) {

            ast_free(node);

            return NULL;
        }

        skip_newlines(parser);

        if (!match(parser, TOKEN_RBRACKET)) {

            parser_error(
                parser,
                "Se esperaba ']' para cerrar el índice."
            );

            ast_free(node);
            ast_free(index);

            return NULL;
        }

        ASTNode *indexed =
            ast_index(node, index);

        if (indexed == NULL) {

            ast_free(node);
            ast_free(index);

            return NULL;
        }

        node = indexed;
    }

    return node;
}

/*
 * NIVEL 2
 *
 * no
 * -  (menos unario)
 *
 * no activo
 * no (edad es mayor que 18)
 * -5
 * -(5 + 3)
 *
 * Este nivel esta POR ENCIMA de
 * '* /', asi que:
 *
 * -5 * 2   →  (-5) * 2
 * 2 * -4   →  2 * (-4)
 * -5 + 10  →  (-5) + 10
 *
 * Y como se llama a si mismo, los
 * signos se pueden encadenar:
 *
 * --5  →  -(-5)  →  5
 *
 * La resta binaria NO se ve
 * afectada: parse_additive mira el
 * '-' DESPUES de tener un operando
 * a la izquierda, y aqui solo se
 * mira al empezar uno.
 */
static ASTNode *parse_unary(Parser *parser) {

    if (match(parser, TOKEN_NO)) {

        ASTNode *operand =
            parse_unary(parser);

        if (operand == NULL) {
            return NULL;
        }

        return make_unary(OP_NOT, operand);
    }

    if (check(parser, TOKEN_MINUS)) {

        /*
         * '-' pegado a un literal
         * entero se pliega en UN solo
         * literal negativo.
         *
         * Es lo que hace que
         * -2147483648 sea valido
         * (cabe en int) mientras
         * 2147483648 por si solo no
         * lo es. Sin esto habria que
         * construir primero el
         * positivo, que no cabe.
         *
         * Cualquier otra cosa sigue
         * el camino normal de
         * OP_NEGATE:
         *
         * -x, -(...), -3.14, --5
         */

        Token *next = peek_token(parser, 1);

        if (
            next != NULL &&
            next->type == TOKEN_NUMBER
        ) {

            advance(parser);

            Token *literal =
                current_token(parser);

            advance(parser);

            return make_number_literal(literal, 1);
        }

        advance(parser);

        ASTNode *operand =
            parse_unary(parser);

        if (operand == NULL) {
            return NULL;
        }

        return make_unary(OP_NEGATE, operand);
    }

    return parse_postfix(parser);
}

/*
 * NIVEL 3
 *
 * * /
 */
static ASTNode *parse_multiplicative(
    Parser *parser
) {

    ASTNode *left =
        parse_unary(parser);

    if (left == NULL) {
        return NULL;
    }

    while (
        check(parser, TOKEN_STAR) ||
        check(parser, TOKEN_SLASH) ||
        check(parser, TOKEN_PERCENT)
    ) {

        /*
         * El resto tiene la misma
         * precedencia que multiplicar y
         * dividir, como en el resto de
         * lenguajes.
         */

        BinaryOperator operator;

        if (check(parser, TOKEN_STAR)) {
            operator = OP_MULTIPLY;
        } else if (check(parser, TOKEN_SLASH)) {
            operator = OP_DIVIDE;
        } else {
            operator = OP_MODULO;
        }

        advance(parser);

        ASTNode *right =
            parse_unary(parser);

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        left =
            make_binary(
                left,
                operator,
                right
            );

        if (left == NULL) {
            return NULL;
        }
    }

    return left;
}

/*
 * NIVEL 4
 *
 * + -
 *
 * Como * / están en un nivel
 * más prioritario:
 *
 * 10 + 2 * 3
 *      ↓
 * 10 + (2 * 3)
 *      ↓
 * 16
 */
static ASTNode *parse_additive(
    Parser *parser
) {

    ASTNode *left =
        parse_multiplicative(parser);

    if (left == NULL) {
        return NULL;
    }

    while (
        check(parser, TOKEN_PLUS) ||
        check(parser, TOKEN_MINUS)
    ) {

        BinaryOperator operator =
            check(parser, TOKEN_PLUS)
                ? OP_ADD
                : OP_SUBTRACT;

        advance(parser);

        ASTNode *right =
            parse_multiplicative(parser);

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        left =
            make_binary(
                left,
                operator,
                right
            );

        if (left == NULL) {
            return NULL;
        }
    }

    return left;
}

/*
 * ==========================
 * OPERADORES DE COMPARACIÓN
 * ==========================
 *
 * ¿Empieza aquí una comparación?
 */
static int at_comparison(Parser *parser) {

    return
        check(parser, TOKEN_ES) ||
        check(parser, TOKEN_GREATER) ||
        check(parser, TOKEN_LESS) ||
        check(parser, TOKEN_GREATER_EQUAL) ||
        check(parser, TOKEN_LESS_EQUAL) ||
        check(parser, TOKEN_EQUAL_EQUAL) ||
        check(parser, TOKEN_NOT_EQUAL);
}

/*
 * Lee el operador de comparación.
 *
 * Acepta las dos formas:
 *
 * edad es mayor que 18
 * edad > 18
 *
 * Devuelve 0 si la frase está
 * mal escrita.
 */
static int read_comparison_operator(
    Parser *parser,
    BinaryOperator *operator
) {

    /*
     * ==========================
     * FORMA SIMBÓLICA
     * ==========================
     */

    if (match(parser, TOKEN_GREATER)) {
        *operator = OP_GREATER;
        return 1;
    }

    if (match(parser, TOKEN_LESS)) {
        *operator = OP_LESS;
        return 1;
    }

    if (match(parser, TOKEN_GREATER_EQUAL)) {
        *operator = OP_GREATER_EQUAL;
        return 1;
    }

    if (match(parser, TOKEN_LESS_EQUAL)) {
        *operator = OP_LESS_EQUAL;
        return 1;
    }

    if (match(parser, TOKEN_EQUAL_EQUAL)) {
        *operator = OP_EQUAL;
        return 1;
    }

    if (match(parser, TOKEN_NOT_EQUAL)) {
        *operator = OP_NOT_EQUAL;
        return 1;
    }

    /*
     * ==========================
     * FORMA PSEUDOCÓDIGO
     * ==========================
     *
     * es ...
     */

    if (!match(parser, TOKEN_ES)) {

        parser_error(
            parser,
            "Se esperaba una comparación."
        );

        return 0;
    }

    /*
     * es mayor que
     * es mayor o igual que
     */

    if (match(parser, TOKEN_MAYOR)) {

        if (match(parser, TOKEN_O)) {

            if (!match(parser, TOKEN_IGUAL)) {

                parser_error(
                    parser,
                    "Se esperaba 'igual' después de 'es mayor o'."
                );

                return 0;
            }

            *operator = OP_GREATER_EQUAL;
        }
        else {

            *operator = OP_GREATER;
        }

        if (!match(parser, TOKEN_QUE)) {

            parser_error(
                parser,
                "Se esperaba 'que' en la comparación."
            );

            return 0;
        }

        return 1;
    }

    /*
     * es menor que
     * es menor o igual que
     */

    if (match(parser, TOKEN_MENOR)) {

        if (match(parser, TOKEN_O)) {

            if (!match(parser, TOKEN_IGUAL)) {

                parser_error(
                    parser,
                    "Se esperaba 'igual' después de 'es menor o'."
                );

                return 0;
            }

            *operator = OP_LESS_EQUAL;
        }
        else {

            *operator = OP_LESS;
        }

        if (!match(parser, TOKEN_QUE)) {

            parser_error(
                parser,
                "Se esperaba 'que' en la comparación."
            );

            return 0;
        }

        return 1;
    }

    /*
     * es igual a
     */

    if (match(parser, TOKEN_IGUAL)) {

        if (!match(parser, TOKEN_A)) {

            parser_error(
                parser,
                "Se esperaba 'a' después de 'es igual'."
            );

            return 0;
        }

        *operator = OP_EQUAL;

        return 1;
    }

    /*
     * es diferente de
     */

    if (match(parser, TOKEN_DIFERENTE)) {

        if (!match(parser, TOKEN_DE)) {

            parser_error(
                parser,
                "Se esperaba 'de' después de 'es diferente'."
            );

            return 0;
        }

        *operator = OP_NOT_EQUAL;

        return 1;
    }

    parser_error(
        parser,
        "Se esperaba 'mayor', 'menor', 'igual' o 'diferente' después de 'es'."
    );

    return 0;
}

/*
 * NIVEL 5
 *
 * comparaciones
 *
 * edad es mayor que 18
 * 10 + 5 > 10
 */
static ASTNode *parse_comparison(
    Parser *parser
) {

    ASTNode *left =
        parse_additive(parser);

    if (left == NULL) {
        return NULL;
    }

    while (at_comparison(parser)) {

        BinaryOperator operator = OP_EQUAL;

        if (
            !read_comparison_operator(
                parser,
                &operator
            )
        ) {
            ast_free(left);
            return NULL;
        }

        ASTNode *right =
            parse_additive(parser);

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        left =
            make_binary(
                left,
                operator,
                right
            );

        if (left == NULL) {
            return NULL;
        }
    }

    return left;
}

/*
 * NIVEL 6
 *
 * y
 */
static ASTNode *parse_and(Parser *parser) {

    ASTNode *left =
        parse_comparison(parser);

    if (left == NULL) {
        return NULL;
    }

    while (match(parser, TOKEN_Y)) {

        ASTNode *right =
            parse_comparison(parser);

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        left =
            make_binary(
                left,
                OP_AND,
                right
            );

        if (left == NULL) {
            return NULL;
        }
    }

    return left;
}

/*
 * NIVEL 7
 *
 * o
 *
 * Es el nivel menos prioritario,
 * así que es la entrada a
 * cualquier expresión.
 */
/*
 * ==========================
 * LIMITE DE ANIDAMIENTO
 * ==========================
 *
 * El parser es de descenso recursivo:
 * cada parentesis, cada corchete y
 * cada llave anidados consumen pila de
 * C. Con miles de ellos la pila se
 * agota y el proceso muere sin
 * mensaje, igual que pasaba con la
 * recursion en el interprete.
 *
 * 500 niveles son mas de los que
 * escribe nadie a mano y dejan un
 * margen enorme antes del limite real.
 */

#define LIMITE_ANIDAMIENTO 500

static int nivel_anidamiento = 0;

static ASTNode *parse_expression(Parser *parser) {

    ASTNode *resultado;

    if (nivel_anidamiento >= LIMITE_ANIDAMIENTO) {

        parser_error(
            parser,
            "La expresion anida demasiados niveles."
        );

        return NULL;
    }

    nivel_anidamiento++;

    resultado = parse_expression_interna(parser);

    nivel_anidamiento--;

    return resultado;
}

static ASTNode *parse_expression_interna(Parser *parser) {

    ASTNode *left =
        parse_and(parser);

    if (left == NULL) {
        return NULL;
    }

    while (match(parser, TOKEN_O)) {

        ASTNode *right =
            parse_and(parser);

        if (right == NULL) {
            ast_free(left);
            return NULL;
        }

        left =
            make_binary(
                left,
                OP_OR,
                right
            );

        if (left == NULL) {
            return NULL;
        }
    }

    return left;
}

/*
 * ==========================
 * INSTRUCCIONES
 * ==========================
 */

static ASTNode *parse_statement(Parser *parser);

/*
 * DECLARACIÓN DE VARIABLE
 *
 * variable x = 10
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
        (
            name->type != TOKEN_IDENTIFIER &&
            !is_contextual_name(name->type)
        )
    ) {

        /*
         * Si lo que hay es una palabra
         * del lenguaje, decirlo. 'y',
         * 'o' y 'no' son los operadores
         * logicos, y a la vez nombres
         * muy tentadores para una
         * variable: sin este mensaje,
         * "se esperaba el nombre" cuando
         * acabas de escribir uno no hay
         * quien lo entienda.
         */

        {
            const char *reservada =
                name == NULL
                    ? NULL
                    : lexer_palabra_reservada(name->type);

            if (reservada != NULL) {

                char detalle[160];

                snprintf(
                    detalle,
                    sizeof(detalle),
                    "'%s' es una palabra reservada del lenguaje "
                    "y no puede ser el nombre de una variable.",
                    reservada
                );

                parser_error(parser, detalle);

                return NULL;
            }
        }

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
     * Fin de instrucción
     */

    if (!expect_terminator(parser)) {

        ast_free(value);

        return NULL;
    }

    return ast_variable(
        name->value,
        value
    );
}

/*
 * ASIGNACIÓN
 *
 * edad = 25
 * contador = contador + 1
 * edad = (edad + 5) * 2
 *
 * parse_statement ya comprobó
 * que venimos de:
 *
 * IDENTIFIER '='
 */
static ASTNode *parse_assignment(
    Parser *parser
) {

    Token *name =
        current_token(parser);

    /*
     * nombre
     */

    advance(parser);

    /*
     * =
     */

    advance(parser);

    /*
     * Valor
     *
     * Usa la misma expresión que
     * el resto del lenguaje, así
     * que la precedencia ya está
     * resuelta.
     */

    ASTNode *value =
        parse_expression(parser);

    if (value == NULL) {
        return NULL;
    }

    if (!expect_terminator(parser)) {

        ast_free(value);

        return NULL;
    }

    return ast_assignment(
        name->value,
        value
    );
}

/*
 * IMPRIMIR
 *
 * Las dos formas son válidas:
 *
 * imprimir "Hola"
 * imprimir(x)
 *
 * Los paréntesis los resuelve
 * parse_primary, así que aquí
 * basta con leer una expresión.
 */
static ASTNode *parse_print(
    Parser *parser
) {

    ASTNode *expression =
        parse_expression(parser);

    if (expression == NULL) {
        return NULL;
    }

    if (!expect_terminator(parser)) {

        ast_free(expression);

        return NULL;
    }

    return ast_print(expression);
}

/*
 * BLOQUE
 *
 * Lista de instrucciones hasta
 * 'sino' o 'fin'.
 *
 * Reutiliza AST_PROGRAM.
 */
static ASTNode *parse_block(
    Parser *parser
) {

    ASTNode *block = ast_program();

    if (block == NULL) {
        return NULL;
    }

    for (;;) {

        skip_terminators(parser);

        if (
            check(parser, TOKEN_FIN) ||
            check(parser, TOKEN_SINO) ||
            check(parser, TOKEN_EOF) ||
            current_token(parser) == NULL
        ) {
            break;
        }

        ASTNode *statement =
            parse_statement(parser);

        if (statement == NULL) {

            ast_free(block);

            return NULL;
        }

        ast_program_add(block, statement);
    }

    return block;
}

/*
 * SI / SINO / FIN
 *
 * si (edad es mayor que 10)
 *     imprimir "Mayor que 10"
 * sino
 *     imprimir "No"
 * fin
 */
static ASTNode *parse_if(
    Parser *parser
) {

    /*
     * Ya consumimos:
     *
     * si
     *
     * La condición puede ir con
     * paréntesis o sin ellos.
     */

    ASTNode *condition =
        parse_expression(parser);

    if (condition == NULL) {
        return NULL;
    }

    if (!expect_terminator(parser)) {

        ast_free(condition);

        return NULL;
    }

    /*
     * Cuerpo del si
     */

    ASTNode *then_branch =
        parse_block(parser);

    if (then_branch == NULL) {

        ast_free(condition);

        return NULL;
    }

    /*
     * sino
     */

    ASTNode *else_branch = NULL;

    if (match(parser, TOKEN_SINO)) {

        /*
         * ==========================
         * sino si
         * ==========================
         *
         * 'sino si' se trata como un
         * 'si' completo metido dentro
         * del sino, que es justo lo que
         * habia que escribir a mano
         * antes:
         *
         *   sino
         *       si ...
         *       fin
         *   fin
         *
         * La diferencia es que ese 'si'
         * interior se come el 'fin' de
         * los dos, asi que una cadena de
         * cinco ramas lleva un 'fin' y
         * no cinco.
         *
         * parse_if() ya exige su propio
         * 'fin', asi que aqui se retorna
         * directamente sin pedir otro.
         */

        if (check(parser, TOKEN_SI)) {

            /*
             * parse_if() da por hecho
             * que el 'si' ya se consumio,
             * igual que cuando la llama
             * parse_statement().
             */

            advance(parser);

            ASTNode *anidado =
                parse_if(parser);

            if (anidado == NULL) {

                ast_free(condition);
                ast_free(then_branch);

                return NULL;
            }

            /*
             * El 'si' anidado va dentro
             * de un bloque, no pelado:
             * execute_block() solo
             * acepta bloques, y con un
             * nodo suelto falla sin
             * decir nada.
             *
             * Asi el arbol queda
             * identico al de escribir el
             * anidamiento a mano.
             */

            ASTNode *envoltorio = ast_program();

            if (envoltorio == NULL) {

                ast_free(condition);
                ast_free(then_branch);
                ast_free(anidado);

                return NULL;
            }

            ast_program_add(envoltorio, anidado);

            return ast_if(
                condition,
                then_branch,
                envoltorio
            );
        }

        else_branch =
            parse_block(parser);

        if (else_branch == NULL) {

            ast_free(condition);
            ast_free(then_branch);

            return NULL;
        }
    }

    /*
     * fin
     */

    if (!match(parser, TOKEN_FIN)) {

        parser_error(
            parser,
            "Se esperaba 'fin' para cerrar el 'si'."
        );

        ast_free(condition);
        ast_free(then_branch);
        ast_free(else_branch);

        return NULL;
    }

    if (!expect_terminator(parser)) {

        ast_free(condition);
        ast_free(then_branch);
        ast_free(else_branch);

        return NULL;
    }

    return ast_if(
        condition,
        then_branch,
        else_branch
    );
}

/*
 * MIENTRAS / FIN
 *
 * mientras (contador es menor que 5)
 *     imprimir contador
 *     contador = contador + 1
 * fin
 *
 * Misma forma que 'si': la condición
 * usa parse_expression (con o sin
 * paréntesis) y el cuerpo reutiliza
 * parse_block.
 */
static ASTNode *parse_while(
    Parser *parser
) {

    /*
     * Ya consumimos:
     *
     * mientras
     */

    ASTNode *condition =
        parse_expression(parser);

    if (condition == NULL) {
        return NULL;
    }

    if (!expect_terminator(parser)) {

        ast_free(condition);

        return NULL;
    }

    /*
     * Cuerpo
     *
     * Dentro de él 'romper' y
     * 'continuar' son legales.
     */

    parser->loop_depth++;

    ASTNode *body =
        parse_block(parser);

    parser->loop_depth--;

    if (body == NULL) {

        ast_free(condition);

        return NULL;
    }

    /*
     * fin
     */

    if (!match(parser, TOKEN_FIN)) {

        parser_error(
            parser,
            "Se esperaba 'fin' para cerrar el 'mientras'."
        );

        ast_free(condition);
        ast_free(body);

        return NULL;
    }

    if (!expect_terminator(parser)) {

        ast_free(condition);
        ast_free(body);

        return NULL;
    }

    return ast_while(
        condition,
        body
    );
}

/*
 * ==========================
 * LISTAS DE PARÁMETROS
 * ==========================
 */

static void free_parameter_list(
    char **parameters,
    int count
) {

    if (parameters == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        free(parameters[i]);
    }

    free(parameters);
}

static void free_argument_list(
    ASTNode **arguments,
    int count
) {

    if (arguments == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        ast_free(arguments[i]);
    }

    free(arguments);
}

/*
 * Copia el texto de un token.
 */
static char *duplicate_token_text(
    const Token *token
) {
    size_t length =
        strlen(token->value);

    char *result =
        malloc(length + 1);

    if (result == NULL) {
        return NULL;
    }

    memcpy(result, token->value, length + 1);

    return result;
}

/*
 * PARÁMETROS
 *
 * ()
 * (nombre)
 * (a, b)
 *
 * Se consume desde '(' hasta ')'.
 *
 * Devuelve 0 si hay error de
 * sintaxis; en ese caso no deja
 * nada reservado.
 */
static int parse_parameter_list(
    Parser *parser,
    char ***out_parameters,
    int *out_count
) {

    *out_parameters = NULL;
    *out_count = 0;

    if (!match(parser, TOKEN_LPAREN)) {

        parser_error(
            parser,
            "Se esperaba '(' después del nombre de la función."
        );

        return 0;
    }

    skip_newlines(parser);

    /*
     * Sin parámetros
     */

    if (match(parser, TOKEN_RPAREN)) {
        return 1;
    }

    char **parameters = NULL;
    int count = 0;
    int capacity = 0;

    for (;;) {

        Token *token =
            current_token(parser);

        if (
            token == NULL ||
            (
                token->type != TOKEN_IDENTIFIER &&
                !is_contextual_name(token->type)
            )
        ) {

            parser_error(
                parser,
                "Se esperaba el nombre de un parámetro."
            );

            free_parameter_list(parameters, count);

            return 0;
        }

        advance(parser);

        /*
         * Crecer la lista
         */

        if (count >= capacity) {

            int new_capacity =
                capacity == 0 ? 4 : capacity * 2;

            char **grown =
                realloc(
                    parameters,
                    sizeof(char *) * new_capacity
                );

            if (grown == NULL) {

                free_parameter_list(parameters, count);

                return 0;
            }

            parameters = grown;
            capacity = new_capacity;
        }

        parameters[count] =
            duplicate_token_text(token);

        if (parameters[count] == NULL) {

            free_parameter_list(parameters, count);

            return 0;
        }

        count++;

        skip_newlines(parser);

        /*
         * , → sigue otro parámetro
         */

        if (match(parser, TOKEN_COMMA)) {

            skip_newlines(parser);

            continue;
        }

        break;
    }

    if (!match(parser, TOKEN_RPAREN)) {

        parser_error(
            parser,
            "Se esperaba ',' o ')' en la lista de parámetros."
        );

        free_parameter_list(parameters, count);

        return 0;
    }

    *out_parameters = parameters;
    *out_count = count;

    return 1;
}

/*
 * ARGUMENTOS
 *
 * ()
 * ("Carlos")
 * (10 + 5, edad * 2)
 *
 * Cada argumento es una EXPRESIÓN
 * sin evaluar: se guarda tal cual
 * y se evalúa al ejecutar.
 */
static int parse_argument_list(
    Parser *parser,
    ASTNode ***out_arguments,
    int *out_count
) {

    *out_arguments = NULL;
    *out_count = 0;

    /*
     * El '(' ya se consumió.
     */

    skip_newlines(parser);

    if (match(parser, TOKEN_RPAREN)) {
        return 1;
    }

    ASTNode **arguments = NULL;
    int count = 0;
    int capacity = 0;

    for (;;) {

        ASTNode *argument =
            parse_expression(parser);

        if (argument == NULL) {

            free_argument_list(arguments, count);

            return 0;
        }

        if (count >= capacity) {

            int new_capacity =
                capacity == 0 ? 4 : capacity * 2;

            ASTNode **grown =
                realloc(
                    arguments,
                    sizeof(ASTNode *) * new_capacity
                );

            if (grown == NULL) {

                ast_free(argument);

                free_argument_list(arguments, count);

                return 0;
            }

            arguments = grown;
            capacity = new_capacity;
        }

        arguments[count] = argument;

        count++;

        skip_newlines(parser);

        if (match(parser, TOKEN_COMMA)) {

            skip_newlines(parser);

            continue;
        }

        break;
    }

    if (!match(parser, TOKEN_RPAREN)) {

        parser_error(
            parser,
            "Se esperaba ',' o ')' en la lista de argumentos."
        );

        free_argument_list(arguments, count);

        return 0;
    }

    *out_arguments = arguments;
    *out_count = count;

    return 1;
}

/*
 * DECLARACIÓN DE FUNCIÓN
 *
 * funcion saludar(nombre)
 *     imprimir "Hola " + nombre
 * fin
 *
 * Declarar no ejecuta nada: el
 * cuerpo se guarda tal cual.
 */
static ASTNode *parse_function_declaration(
    Parser *parser
) {

    /*
     * Ya consumimos:
     *
     * funcion
     */

    Token *name =
        current_token(parser);

    if (
        name == NULL ||
        (
            name->type != TOKEN_IDENTIFIER &&
            !is_contextual_name(name->type)
        )
    ) {

        parser_error(
            parser,
            "Se esperaba el nombre de la función."
        );

        return NULL;
    }

    advance(parser);

    /*
     * (parametros)
     */

    char **parameters = NULL;
    int parameter_count = 0;

    if (
        !parse_parameter_list(
            parser,
            &parameters,
            &parameter_count
        )
    ) {
        return NULL;
    }

    if (!expect_terminator(parser)) {

        free_parameter_list(
            parameters,
            parameter_count
        );

        return NULL;
    }

    /*
     * Cuerpo
     *
     * Dentro de él 'retornar' es
     * legal.
     *
     * El contador de bucles se
     * reinicia: si esta declaración
     * está dentro de un 'mientras',
     * su cuerpo NO puede romperlo,
     * porque la función puede
     * llamarse desde cualquier sitio.
     */

    int saved_loop_depth =
        parser->loop_depth;

    parser->loop_depth = 0;
    parser->function_depth++;

    ASTNode *body =
        parse_block(parser);

    parser->function_depth--;
    parser->loop_depth = saved_loop_depth;

    if (body == NULL) {

        free_parameter_list(
            parameters,
            parameter_count
        );

        return NULL;
    }

    /*
     * fin
     */

    if (!match(parser, TOKEN_FIN)) {

        parser_error(
            parser,
            "Se esperaba 'fin' para cerrar la función."
        );

        free_parameter_list(
            parameters,
            parameter_count
        );

        ast_free(body);

        return NULL;
    }

    if (!expect_terminator(parser)) {

        free_parameter_list(
            parameters,
            parameter_count
        );

        ast_free(body);

        return NULL;
    }

    ASTNode *node =
        ast_function_declaration(
            name->value,
            parameters,
            parameter_count,
            body
        );

    /*
     * Si el nodo no se pudo crear,
     * la lista sigue siendo nuestra.
     */

    if (node == NULL) {

        free_parameter_list(
            parameters,
            parameter_count
        );

        ast_free(body);
    }

    return node;
}

/*
 * LLAMADA COMO EXPRESIÓN
 *
 * saludar()
 * sumar(10, 20)
 *
 * NO consume el fin de instrucción,
 * porque puede estar en medio de
 * una expresión:
 *
 * sumar(1, 2) * 3
 *
 * Quien llama ya comprobó que
 * venimos de:
 *
 * IDENTIFIER '('
 */
static ASTNode *parse_call_expression(
    Parser *parser
) {

    Token *name =
        current_token(parser);

    /*
     * nombre
     */

    advance(parser);

    /*
     * (
     */

    advance(parser);

    /*
     * argumentos)
     */

    ASTNode **arguments = NULL;
    int argument_count = 0;

    if (
        !parse_argument_list(
            parser,
            &arguments,
            &argument_count
        )
    ) {
        return NULL;
    }

    ASTNode *node =
        ast_function_call(
            name->value,
            arguments,
            argument_count
        );

    if (node == NULL) {

        free_argument_list(
            arguments,
            argument_count
        );
    }

    return node;
}

/*
 * LLAMADA COMO INSTRUCCIÓN
 *
 * saludar()
 *
 * Es la misma llamada más el fin
 * de instrucción. El valor que
 * devuelva se descarta.
 */
static ASTNode *parse_function_call(
    Parser *parser
) {

    ASTNode *call =
        parse_call_expression(parser);

    if (call == NULL) {
        return NULL;
    }

    if (!expect_terminator(parser)) {

        ast_free(call);

        return NULL;
    }

    return call;
}

/*
 * RETORNAR
 *
 * retornar
 * retornar 10
 * retornar a + b
 * retornar sumar(1, 2)
 */
static ASTNode *parse_return(
    Parser *parser
) {

    /*
     * Ya consumimos:
     *
     * retornar
     */

    if (parser->function_depth <= 0) {

        parser_error(
            parser,
            "'retornar' solo puede utilizarse dentro de una función."
        );

        return NULL;
    }

    /*
     * 'retornar' a secas devuelve
     * nulo.
     */

    if (at_statement_end(parser)) {

        if (!expect_terminator(parser)) {
            return NULL;
        }

        return ast_return(NULL);
    }

    ASTNode *value =
        parse_expression(parser);

    if (value == NULL) {
        return NULL;
    }

    if (!expect_terminator(parser)) {

        ast_free(value);

        return NULL;
    }

    ASTNode *node =
        ast_return(value);

    if (node == NULL) {
        ast_free(value);
    }

    return node;
}

/*
 * PARA CADA / FIN
 *
 * para cada numero en numeros
 *     imprimir numero
 * fin
 *
 * La lista puede ser cualquier
 * expresión, así que también vale:
 *
 * para cada x en crear()
 * para cada x en [1, 2, 3]
 * para cada x en matriz[0]
 */
static ASTNode *parse_for_each(
    Parser *parser
) {

    /*
     * Ya consumimos:
     *
     * para
     */

    if (!match(parser, TOKEN_CADA)) {

        parser_error(
            parser,
            "Se esperaba 'cada' después de 'para'."
        );

        return NULL;
    }

    /*
     * Nombre de la variable del
     * bucle.
     */

    Token *name =
        current_token(parser);

    if (
        name == NULL ||
        (
            name->type != TOKEN_IDENTIFIER &&
            !is_contextual_name(name->type)
        )
    ) {

        {
            const char *reservada =
                name == NULL
                    ? NULL
                    : lexer_palabra_reservada(name->type);

            if (reservada != NULL) {

                char detalle[170];

                snprintf(
                    detalle,
                    sizeof(detalle),
                    "'%s' es una palabra reservada del lenguaje "
                    "y no puede ser el nombre de la variable del bucle.",
                    reservada
                );

                parser_error(parser, detalle);

                return NULL;
            }
        }

        parser_error(
            parser,
            "Se esperaba el nombre de la variable del bucle."
        );

        return NULL;
    }

    advance(parser);

    if (!match(parser, TOKEN_EN)) {

        parser_error(
            parser,
            "Se esperaba 'en' después del nombre en el 'para cada'."
        );

        return NULL;
    }

    /*
     * La lista a recorrer
     */

    ASTNode *iterable =
        parse_expression(parser);

    if (iterable == NULL) {
        return NULL;
    }

    if (!expect_terminator(parser)) {

        ast_free(iterable);

        return NULL;
    }

    /*
     * Cuerpo
     *
     * Es un bucle, así que dentro
     * 'romper' y 'continuar' son
     * legales.
     */

    parser->loop_depth++;

    ASTNode *body =
        parse_block(parser);

    parser->loop_depth--;

    if (body == NULL) {

        ast_free(iterable);

        return NULL;
    }

    /*
     * fin
     */

    if (!match(parser, TOKEN_FIN)) {

        parser_error(
            parser,
            "Se esperaba 'fin' para cerrar el 'para cada'."
        );

        ast_free(iterable);
        ast_free(body);

        return NULL;
    }

    if (!expect_terminator(parser)) {

        ast_free(iterable);
        ast_free(body);

        return NULL;
    }

    ASTNode *node =
        ast_for_each(
            name->value,
            iterable,
            body
        );

    if (node == NULL) {

        ast_free(iterable);
        ast_free(body);
    }

    return node;
}

/*
 * ROMPER / CONTINUAR
 *
 * romper
 * continuar
 *
 * También con ';'.
 *
 * Solo son válidos dentro de un
 * 'mientras', y se comprueba aquí
 * —en el análisis— para que
 * incluso una línea inalcanzable
 * como esta se rechace:
 *
 * si falso
 *     romper
 * fin
 */
static ASTNode *parse_break(
    Parser *parser
) {

    if (parser->loop_depth <= 0) {

        parser_error(
            parser,
            "'romper' solo puede utilizarse dentro de un 'mientras'."
        );

        return NULL;
    }

    if (!expect_terminator(parser)) {
        return NULL;
    }

    return ast_break();
}

static ASTNode *parse_continue(
    Parser *parser
) {

    if (parser->loop_depth <= 0) {

        parser_error(
            parser,
            "'continuar' solo puede utilizarse dentro de un 'mientras'."
        );

        return NULL;
    }

    if (!expect_terminator(parser)) {
        return NULL;
    }

    return ast_continue();
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

    /*
     * si
     */
    if (match(parser, TOKEN_SI)) {

        return parse_if(parser);
    }

    /*
     * mientras
     */
    if (match(parser, TOKEN_MIENTRAS)) {

        return parse_while(parser);
    }

    /*
     * para cada
     */
    if (match(parser, TOKEN_PARA)) {

        return parse_for_each(parser);
    }

    /*
     * funcion
     */
    if (match(parser, TOKEN_FUNCION)) {

        return parse_function_declaration(parser);
    }

    /*
     * retornar
     */
    if (match(parser, TOKEN_RETORNAR)) {

        return parse_return(parser);
    }

    /*
     * romper
     */
    if (match(parser, TOKEN_ROMPER)) {

        return parse_break(parser);
    }

    /*
     * continuar
     */
    if (match(parser, TOKEN_CONTINUAR)) {

        return parse_continue(parser);
    }

    /*
     * Instrucciones que empiezan
     * por un nombre.
     *
     * El token SIGUIENTE decide:
     *
     * edad = 25    →  asignación
     * saludar()    →  llamada
     *
     * Ojo: '=' no es '=='.
     */

    Token *token =
        current_token(parser);

    Token *next =
        peek_token(parser, 1);

    if (
        token != NULL &&
        next != NULL &&
        (
            token->type == TOKEN_IDENTIFIER ||
            is_contextual_name(token->type)
        )
    ) {

        if (next->type == TOKEN_EQUAL) {

            return parse_assignment(parser);
        }

        if (next->type == TOKEN_LPAREN) {

            return parse_function_call(parser);
        }

        /*
         * ASIGNACIÓN POR ÍNDICE
         *
         * numeros[1] = 99
         * matriz[1][0] = 5
         * numeros[i + 1] = 50
         *
         * El índice es una expresión
         * cualquiera, así que no se
         * puede decidir mirando dos
         * tokens: se analiza la parte
         * indexada entera y luego se
         * exige '='.
         *
         * Como se exige '=' y no
         * '==', numeros[1] == 99 no
         * se confunde con una
         * asignación.
         */

        if (next->type == TOKEN_LBRACKET) {

            ASTNode *target =
                parse_postfix(parser);

            if (target == NULL) {
                return NULL;
            }

            if (!match(parser, TOKEN_EQUAL)) {

                parser_error(
                    parser,
                    "Se esperaba '=' después del índice."
                );

                ast_free(target);

                return NULL;
            }

            ASTNode *value =
                parse_expression(parser);

            if (value == NULL) {

                ast_free(target);

                return NULL;
            }

            if (!expect_terminator(parser)) {

                ast_free(target);
                ast_free(value);

                return NULL;
            }

            ASTNode *node =
                ast_index_assignment(
                    target,
                    value
                );

            if (node == NULL) {

                ast_free(target);
                ast_free(value);
            }

            return node;
        }
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

    for (;;) {

        skip_terminators(parser);

        if (
            check(parser, TOKEN_EOF) ||
            current_token(parser) == NULL
        ) {
            break;
        }

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
