#ifndef TZLANG_AST_H
#define TZLANG_AST_H

typedef enum {
    AST_PROGRAM,

    AST_NUMBER,
    AST_DECIMAL,
    AST_STRING,
    AST_BOOLEAN,
    AST_NULL,
    AST_IDENTIFIER,

    AST_BINARY,
    AST_UNARY,

    AST_VARIABLE_DECLARATION,
    AST_ASSIGNMENT,
    AST_PRINT,
    AST_IF,
    AST_WHILE,
    AST_FUNCTION_DECLARATION,
    AST_FUNCTION_CALL,
    AST_RETURN,

    /*
     * romper / continuar
     *
     * No guardan datos: toda la
     * información está en el tipo.
     */

    AST_BREAK,
    AST_CONTINUE,

    /*
     * Listas
     *
     * [10, 20, 30]     → AST_LIST
     * numeros[0]       → AST_INDEX
     * numeros[0] = 99  → AST_INDEX_ASSIGNMENT
     */

    AST_LIST,
    AST_INDEX,
    AST_INDEX_ASSIGNMENT,

    /*
     * para cada X en LISTA
     */

    AST_FOR_EACH,

    /*
     * {"nombre": "Carlos"}
     *
     * El acceso persona["nombre"]
     * y la asignacion reutilizan
     * AST_INDEX / AST_INDEX_ASSIGNMENT:
     * la diferencia entre lista y
     * diccionario se resuelve en
     * runtime, no en la gramatica.
     */

    AST_DICTIONARY
} ASTNodeType;

/*
 * ==========================
 * OPERADORES BINARIOS
 * ==========================
 *
 * La sintaxis visible es
 * pseudocódigo en español:
 *
 * "es mayor que"        → OP_GREATER
 * "es mayor o igual que" → OP_GREATER_EQUAL
 * "es igual a"          → OP_EQUAL
 * "es diferente de"     → OP_NOT_EQUAL
 * "y"                   → OP_AND
 * "o"                   → OP_OR
 *
 * Internamente todo se reduce
 * a estos operadores.
 */

typedef enum {

    /*
     * Aritmética
     */

    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,

    /*
     * Comparación
     */

    OP_GREATER,
    OP_LESS,
    OP_GREATER_EQUAL,
    OP_LESS_EQUAL,
    OP_EQUAL,
    OP_NOT_EQUAL,

    /*
     * Lógicos
     */

    OP_AND,
    OP_OR

} BinaryOperator;

/*
 * ==========================
 * OPERADORES UNARIOS
 * ==========================
 *
 * "no" → OP_NOT
 * "-"  → OP_NEGATE
 */

typedef enum {
    OP_NOT,
    OP_NEGATE
} UnaryOperator;

typedef struct ASTNode ASTNode;

struct ASTNode {
    ASTNodeType type;

    union {

        int number;

        double decimal;

        char *string;

        int boolean;

        char *identifier;

        struct {
            ASTNode *left;
            ASTNode *right;
            BinaryOperator operator;
        } binary;

        struct {
            ASTNode *operand;
            UnaryOperator operator;
        } unary;

        struct {
            char *name;
            ASTNode *value;
        } variable;

        struct {
            char *name;
            ASTNode *value;
        } assignment;

        ASTNode *print;

        /*
         * retornar expresion
         *
         * NULL cuando es 'retornar'
         * a secas: devuelve nulo.
         */

        ASTNode *return_value;

        struct {
            ASTNode *condition;
            ASTNode *then_branch;
            ASTNode *else_branch;
        } if_statement;

        struct {
            ASTNode *condition;
            ASTNode *body;
        } while_statement;

        struct {
            char *name;
            char **parameters;
            int parameter_count;
            ASTNode *body;
        } function_declaration;

        struct {
            char *name;
            ASTNode **arguments;
            int argument_count;
        } function_call;

        /*
         * [10, 20, 30]
         *
         * Los elementos son
         * expresiones SIN evaluar.
         */

        struct {
            ASTNode **elements;
            int element_count;
        } list;

        /*
         * numeros[0]
         *
         * 'object' puede ser a su vez
         * otro AST_INDEX, y así
         * matriz[1][0] funciona sin
         * ningún caso especial.
         */

        struct {
            ASTNode *object;
            ASTNode *index;
        } index;

        /*
         * numeros[0] = 99
         *
         * 'target' es siempre un
         * AST_INDEX.
         */

        struct {
            ASTNode *target;
            ASTNode *value;
        } index_assignment;

        /*
         * para cada numero en numeros
         *     ...
         * fin
         *
         * 'iterable' es una expresión
         * cualquiera que produzca una
         * lista.
         *
         * El body reutiliza AST_PROGRAM.
         */

        struct {
            char *variable;
            ASTNode *iterable;
            ASTNode *body;
        } for_each;

        /*
         * {"a": 1, "b": 2}
         *
         * Claves y valores son
         * expresiones SIN evaluar,
         * en arrays paralelos.
         *
         * La clave debe producir un
         * texto al evaluarse.
         */

        struct {
            ASTNode **keys;
            ASTNode **values;
            int pair_count;
        } dictionary;

        struct {
            ASTNode **statements;
            int count;
            int capacity;
        } program;

    } data;
};

/*
 * PROGRAM
 *
 * También se usa para los
 * bloques de 'si' y 'sino'.
 */

ASTNode *ast_program(void);

void ast_program_add(
    ASTNode *program,
    ASTNode *statement
);

/*
 * VALUES
 */

ASTNode *ast_number(int value);

ASTNode *ast_decimal(double value);

ASTNode *ast_string(const char *value);

ASTNode *ast_boolean(int value);

ASTNode *ast_null(void);

ASTNode *ast_identifier(const char *name);

/*
 * EXPRESSIONS
 */

ASTNode *ast_binary(
    ASTNode *left,
    BinaryOperator operator,
    ASTNode *right
);

ASTNode *ast_unary(
    UnaryOperator operator,
    ASTNode *operand
);

/*
 * STATEMENTS
 */

ASTNode *ast_variable(
    const char *name,
    ASTNode *value
);

/*
 * ASIGNACIÓN
 *
 * Modifica una variable que YA
 * existe:
 *
 * edad = 25
 * contador = contador + 1
 *
 * A diferencia de ast_variable,
 * no declara nada nuevo.
 */

ASTNode *ast_assignment(
    const char *name,
    ASTNode *value
);

ASTNode *ast_print(
    ASTNode *expression
);

/*
 * retornar expresion
 *
 * 'value' puede ser NULL:
 *
 * retornar        → nulo
 * retornar 10     → 10
 */

ASTNode *ast_return(
    ASTNode *value
);

/*
 * romper    → sale del mientras
 * continuar → siguiente vuelta
 *
 * Solo válidos dentro de un
 * 'mientras' (lo comprueba el
 * parser).
 */

ASTNode *ast_break(void);

ASTNode *ast_continue(void);

/*
 * [10, 20, 30]
 *
 * MEMORIA: misma regla que los
 * argumentos de una llamada. En
 * caso de ÉXITO el nodo se adueña
 * de 'elements' y de cada ASTNode;
 * si devuelve NULL, la lista sigue
 * siendo de quien llama.
 *
 * elements puede ser NULL si
 * element_count es 0.
 */

ASTNode *ast_list(
    ASTNode **elements,
    int element_count
);

/*
 * numeros[0]
 */

ASTNode *ast_index(
    ASTNode *object,
    ASTNode *index
);

/*
 * numeros[0] = 99
 *
 * 'target' debe ser un AST_INDEX.
 */

ASTNode *ast_index_assignment(
    ASTNode *target,
    ASTNode *value
);

/*
 * para cada numero en numeros
 *     body
 * fin
 */

ASTNode *ast_for_each(
    const char *variable,
    ASTNode *iterable,
    ASTNode *body
);

/*
 * {"a": 1, "b": 2}
 *
 * MEMORIA: misma regla que la
 * lista. En caso de EXITO el nodo
 * se aduena de 'keys', 'values' y
 * de cada ASTNode; si devuelve
 * NULL siguen siendo de quien
 * llama.
 *
 * Ambos arrays pueden ser NULL si
 * pair_count es 0.
 */

ASTNode *ast_dictionary(
    ASTNode **keys,
    ASTNode **values,
    int pair_count
);

/*
 * si (condicion)
 *     then_branch
 * sino
 *     else_branch
 * fin
 *
 * else_branch puede ser NULL.
 */

ASTNode *ast_if(
    ASTNode *condition,
    ASTNode *then_branch,
    ASTNode *else_branch
);

/*
 * mientras (condicion)
 *     body
 * fin
 *
 * El body reutiliza AST_PROGRAM,
 * igual que los bloques del 'si'.
 */

ASTNode *ast_while(
    ASTNode *condition,
    ASTNode *body
);

/*
 * funcion nombre(a, b)
 *     body
 * fin
 *
 * El body reutiliza AST_PROGRAM.
 *
 * Declarar NO ejecuta: el nodo
 * solo guarda el cuerpo.
 *
 * MEMORIA:
 *
 * En caso de ÉXITO el nodo se
 * adueña de 'parameters' y de
 * cada string que contiene, y
 * los libera en ast_free().
 *
 * Si devuelve NULL, la lista
 * sigue siendo de quien llama.
 *
 * parameters puede ser NULL si
 * parameter_count es 0.
 */

ASTNode *ast_function_declaration(
    const char *name,
    char **parameters,
    int parameter_count,
    ASTNode *body
);

/*
 * nombre(10 + 5, edad)
 *
 * Los argumentos son expresiones
 * SIN evaluar: se evalúan al
 * ejecutar la llamada.
 *
 * MEMORIA:
 *
 * Misma regla que arriba. En caso
 * de éxito el nodo se adueña de
 * 'arguments' y de cada ASTNode,
 * y los libera con ast_free().
 */

ASTNode *ast_function_call(
    const char *name,
    ASTNode **arguments,
    int argument_count
);

/*
 * MEMORY
 */

void ast_free(ASTNode *node);

/*
 * DEBUG
 */

void ast_print_tree(
    const ASTNode *node,
    int indentation
);

#endif
