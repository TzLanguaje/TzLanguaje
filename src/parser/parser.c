#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

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

static void skip_terminators(Parser *parser) {

    while (
        check(parser, TOKEN_NEWLINE) ||
        check(parser, TOKEN_SEMICOLON)
    ) {
        advance(parser);
    }
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
        check(parser, TOKEN_EOF) ||
        check(parser, TOKEN_FIN) ||
        check(parser, TOKEN_SINO) ||
        current_token(parser) == NULL
    ) {
        return 1;
    }

    if (
        check(parser, TOKEN_NEWLINE) ||
        check(parser, TOKEN_SEMICOLON)
    ) {
        skip_terminators(parser);

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
        type == TOKEN_DE;
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

static ASTNode *parse_expression(Parser *parser);

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

        ASTNode *expression =
            parse_expression(parser);

        if (expression == NULL) {
            return NULL;
        }

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
     * NUMBER
     * ==========================
     *
     * 19
     */

    if (match(parser, TOKEN_NUMBER)) {

        int value =
            atoi(token->value);

        return ast_number(value);
    }

    /*
     * ==========================
     * DECIMAL
     * ==========================
     *
     * 1.78
     */

    if (match(parser, TOKEN_DECIMAL)) {

        double value =
            atof(token->value);

        return ast_decimal(value);
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
 * NIVEL 2
 *
 * no
 *
 * no activo
 * no (edad es mayor que 18)
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

    return parse_primary(parser);
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
        check(parser, TOKEN_SLASH)
    ) {

        BinaryOperator operator =
            check(parser, TOKEN_STAR)
                ? OP_MULTIPLY
                : OP_DIVIDE;

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
static ASTNode *parse_expression(Parser *parser) {

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
     * asignación
     *
     * edad = 25
     *
     * Se distingue de una expresión
     * mirando el token siguiente:
     * tiene que ser '=' (no '==').
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
        ) &&
        next->type == TOKEN_EQUAL
    ) {

        return parse_assignment(parser);
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
