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
    AST_WHILE
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
 */

typedef enum {
    OP_NOT
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
