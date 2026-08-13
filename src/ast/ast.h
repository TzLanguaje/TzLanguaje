#ifndef TZLANG_AST_H
#define TZLANG_AST_H

typedef enum {
    AST_PROGRAM,

    AST_NUMBER,
    AST_DECIMAL,
    AST_STRING,
    AST_BOOLEAN,
    AST_IDENTIFIER,

    AST_BINARY,

    AST_VARIABLE_DECLARATION,
    AST_PRINT
} ASTNodeType;

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
            char operator;
        } binary;

        struct {
            char *name;
            ASTNode *value;
        } variable;

        ASTNode *print;

        struct {
            ASTNode **statements;
            int count;
            int capacity;
        } program;

    } data;
};

/*
 * PROGRAM
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

ASTNode *ast_identifier(const char *name);

/*
 * EXPRESSIONS
 */

ASTNode *ast_binary(
    ASTNode *left,
    char operator,
    ASTNode *right
);

/*
 * STATEMENTS
 */

ASTNode *ast_variable(
    const char *name,
    ASTNode *value
);

ASTNode *ast_print(
    ASTNode *expression
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