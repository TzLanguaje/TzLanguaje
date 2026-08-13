#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ==========================
 * UTILIDADES
 * ==========================
 */

static char *copy_string(const char *source) {
    size_t length = strlen(source);

    char *result = malloc(length + 1);

    if (result == NULL) {
        return NULL;
    }

    strcpy(result, source);

    return result;
}

static ASTNode *create_node(ASTNodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL) {
        return NULL;
    }

    node->type = type;

    return node;
}

/*
 * ==========================
 * PROGRAM
 * ==========================
 */

ASTNode *ast_program(void) {
    ASTNode *node = create_node(AST_PROGRAM);

    if (node == NULL) {
        return NULL;
    }

    node->data.program.count = 0;
    node->data.program.capacity = 8;

    node->data.program.statements =
        malloc(
            sizeof(ASTNode *) *
            node->data.program.capacity
        );

    if (node->data.program.statements == NULL) {
        free(node);
        return NULL;
    }

    return node;
}

void ast_program_add(
    ASTNode *program,
    ASTNode *statement
) {
    if (
        program == NULL ||
        statement == NULL ||
        program->type != AST_PROGRAM
    ) {
        return;
    }

    if (
        program->data.program.count >=
        program->data.program.capacity
    ) {
        int new_capacity =
            program->data.program.capacity * 2;

        ASTNode **new_statements =
            realloc(
                program->data.program.statements,
                sizeof(ASTNode *) * new_capacity
            );

        if (new_statements == NULL) {
            return;
        }

        program->data.program.statements =
            new_statements;

        program->data.program.capacity =
            new_capacity;
    }

    program->data.program.statements[
        program->data.program.count
    ] = statement;

    program->data.program.count++;
}

/*
 * ==========================
 * VALUES
 * ==========================
 */

ASTNode *ast_number(int value) {
    ASTNode *node = create_node(AST_NUMBER);

    if (node == NULL) {
        return NULL;
    }

    node->data.number = value;

    return node;
}

ASTNode *ast_decimal(double value) {
    ASTNode *node = create_node(AST_DECIMAL);

    if (node == NULL) {
        return NULL;
    }

    node->data.decimal = value;

    return node;
}

ASTNode *ast_string(const char *value) {
    ASTNode *node = create_node(AST_STRING);

    if (node == NULL) {
        return NULL;
    }

    node->data.string = copy_string(value);

    if (node->data.string == NULL) {
        free(node);
        return NULL;
    }

    return node;
}

ASTNode *ast_boolean(int value) {
    ASTNode *node = create_node(AST_BOOLEAN);

    if (node == NULL) {
        return NULL;
    }

    node->data.boolean = value ? 1 : 0;

    return node;
}

ASTNode *ast_null(void) {
    return create_node(AST_NULL);
}

ASTNode *ast_identifier(const char *name) {
    ASTNode *node = create_node(AST_IDENTIFIER);

    if (node == NULL) {
        return NULL;
    }

    node->data.identifier = copy_string(name);

    if (node->data.identifier == NULL) {
        free(node);
        return NULL;
    }

    return node;
}

/*
 * ==========================
 * EXPRESSIONS
 * ==========================
 */

ASTNode *ast_binary(
    ASTNode *left,
    BinaryOperator operator,
    ASTNode *right
) {
    ASTNode *node = create_node(AST_BINARY);

    if (node == NULL) {
        return NULL;
    }

    node->data.binary.left = left;
    node->data.binary.operator = operator;
    node->data.binary.right = right;

    return node;
}

ASTNode *ast_unary(
    UnaryOperator operator,
    ASTNode *operand
) {
    ASTNode *node = create_node(AST_UNARY);

    if (node == NULL) {
        return NULL;
    }

    node->data.unary.operator = operator;
    node->data.unary.operand = operand;

    return node;
}

/*
 * ==========================
 * STATEMENTS
 * ==========================
 */

ASTNode *ast_variable(
    const char *name,
    ASTNode *value
) {
    ASTNode *node =
        create_node(AST_VARIABLE_DECLARATION);

    if (node == NULL) {
        return NULL;
    }

    node->data.variable.name =
        copy_string(name);

    if (node->data.variable.name == NULL) {
        free(node);
        return NULL;
    }

    node->data.variable.value = value;

    return node;
}

ASTNode *ast_assignment(
    const char *name,
    ASTNode *value
) {
    ASTNode *node =
        create_node(AST_ASSIGNMENT);

    if (node == NULL) {
        return NULL;
    }

    node->data.assignment.name =
        copy_string(name);

    if (node->data.assignment.name == NULL) {
        free(node);
        return NULL;
    }

    node->data.assignment.value = value;

    return node;
}

ASTNode *ast_print(ASTNode *expression) {
    ASTNode *node = create_node(AST_PRINT);

    if (node == NULL) {
        return NULL;
    }

    node->data.print = expression;

    return node;
}

ASTNode *ast_if(
    ASTNode *condition,
    ASTNode *then_branch,
    ASTNode *else_branch
) {
    ASTNode *node = create_node(AST_IF);

    if (node == NULL) {
        return NULL;
    }

    node->data.if_statement.condition =
        condition;

    node->data.if_statement.then_branch =
        then_branch;

    node->data.if_statement.else_branch =
        else_branch;

    return node;
}

ASTNode *ast_while(
    ASTNode *condition,
    ASTNode *body
) {
    ASTNode *node = create_node(AST_WHILE);

    if (node == NULL) {
        return NULL;
    }

    node->data.while_statement.condition =
        condition;

    node->data.while_statement.body =
        body;

    return node;
}

/*
 * ==========================
 * NOMBRE DE LOS OPERADORES
 * ==========================
 */

static const char *binary_operator_name(
    BinaryOperator operator
) {

    switch (operator) {

        case OP_ADD:
            return "+";

        case OP_SUBTRACT:
            return "-";

        case OP_MULTIPLY:
            return "*";

        case OP_DIVIDE:
            return "/";

        case OP_GREATER:
            return ">";

        case OP_LESS:
            return "<";

        case OP_GREATER_EQUAL:
            return ">=";

        case OP_LESS_EQUAL:
            return "<=";

        case OP_EQUAL:
            return "==";

        case OP_NOT_EQUAL:
            return "!=";

        case OP_AND:
            return "y";

        case OP_OR:
            return "o";

        default:
            return "?";
    }
}

static const char *unary_operator_name(
    UnaryOperator operator
) {

    switch (operator) {

        case OP_NOT:
            return "no";

        default:
            return "?";
    }
}

/*
 * ==========================
 * AST TREE PRINTING
 * ==========================
 */

static void print_indent(int indentation) {
    for (int i = 0; i < indentation; i++) {
        printf("  ");
    }
}

void ast_print_tree(
    const ASTNode *node,
    int indentation
) {
    if (node == NULL) {
        return;
    }

    print_indent(indentation);

    switch (node->type) {

        /*
         * PROGRAM
         */

        case AST_PROGRAM:

            printf("PROGRAM\n");

            for (
                int i = 0;
                i < node->data.program.count;
                i++
            ) {
                ast_print_tree(
                    node->data.program.statements[i],
                    indentation + 1
                );
            }

            break;

        /*
         * NUMBER
         */

        case AST_NUMBER:

            printf(
                "NUMBER: %d\n",
                node->data.number
            );

            break;

        /*
         * DECIMAL
         */

        case AST_DECIMAL:

            printf(
                "DECIMAL: %g\n",
                node->data.decimal
            );

            break;

        /*
         * STRING
         */

        case AST_STRING:

            printf(
                "STRING: \"%s\"\n",
                node->data.string
            );

            break;

        /*
         * BOOLEAN
         */

        case AST_BOOLEAN:

            printf(
                "BOOLEAN: %s\n",
                node->data.boolean
                    ? "verdadero"
                    : "falso"
            );

            break;

        /*
         * NULL
         */

        case AST_NULL:

            printf("NULL: nulo\n");

            break;

        /*
         * IDENTIFIER
         */

        case AST_IDENTIFIER:

            printf(
                "IDENTIFIER: %s\n",
                node->data.identifier
            );

            break;

        /*
         * BINARY
         */

        case AST_BINARY:

            printf(
                "BINARY: %s\n",
                binary_operator_name(
                    node->data.binary.operator
                )
            );

            ast_print_tree(
                node->data.binary.left,
                indentation + 1
            );

            ast_print_tree(
                node->data.binary.right,
                indentation + 1
            );

            break;

        /*
         * UNARY
         */

        case AST_UNARY:

            printf(
                "UNARY: %s\n",
                unary_operator_name(
                    node->data.unary.operator
                )
            );

            ast_print_tree(
                node->data.unary.operand,
                indentation + 1
            );

            break;

        /*
         * VARIABLE
         */

        case AST_VARIABLE_DECLARATION:

            printf(
                "VARIABLE: %s\n",
                node->data.variable.name
            );

            ast_print_tree(
                node->data.variable.value,
                indentation + 1
            );

            break;

        /*
         * ASSIGNMENT
         */

        case AST_ASSIGNMENT:

            printf(
                "ASSIGNMENT: %s\n",
                node->data.assignment.name
            );

            ast_print_tree(
                node->data.assignment.value,
                indentation + 1
            );

            break;

        /*
         * PRINT
         */

        case AST_PRINT:

            printf("PRINT\n");

            ast_print_tree(
                node->data.print,
                indentation + 1
            );

            break;

        /*
         * IF
         */

        case AST_IF:

            printf("IF\n");

            print_indent(indentation + 1);
            printf("CONDITION\n");

            ast_print_tree(
                node->data.if_statement.condition,
                indentation + 2
            );

            print_indent(indentation + 1);
            printf("THEN\n");

            ast_print_tree(
                node->data.if_statement.then_branch,
                indentation + 2
            );

            if (
                node->data.if_statement.else_branch
                != NULL
            ) {

                print_indent(indentation + 1);
                printf("ELSE\n");

                ast_print_tree(
                    node->data.if_statement.else_branch,
                    indentation + 2
                );
            }

            break;

        /*
         * WHILE
         */

        case AST_WHILE:

            printf("WHILE\n");

            print_indent(indentation + 1);
            printf("CONDITION\n");

            ast_print_tree(
                node->data.while_statement.condition,
                indentation + 2
            );

            print_indent(indentation + 1);
            printf("BODY\n");

            ast_print_tree(
                node->data.while_statement.body,
                indentation + 2
            );

            break;

        /*
         * UNKNOWN
         */

        default:

            printf("UNKNOWN\n");

            break;
    }
}

/*
 * ==========================
 * AST MEMORY MANAGEMENT
 * ==========================
 */

void ast_free(ASTNode *node) {

    if (node == NULL) {
        return;
    }

    switch (node->type) {

        /*
         * PROGRAM
         */

        case AST_PROGRAM:

            for (
                int i = 0;
                i < node->data.program.count;
                i++
            ) {
                ast_free(
                    node->data.program.statements[i]
                );
            }

            free(
                node->data.program.statements
            );

            break;

        /*
         * STRING
         */

        case AST_STRING:

            free(node->data.string);

            break;

        /*
         * IDENTIFIER
         */

        case AST_IDENTIFIER:

            free(node->data.identifier);

            break;

        /*
         * BINARY
         */

        case AST_BINARY:

            ast_free(
                node->data.binary.left
            );

            ast_free(
                node->data.binary.right
            );

            break;

        /*
         * UNARY
         */

        case AST_UNARY:

            ast_free(
                node->data.unary.operand
            );

            break;

        /*
         * VARIABLE
         */

        case AST_VARIABLE_DECLARATION:

            free(
                node->data.variable.name
            );

            ast_free(
                node->data.variable.value
            );

            break;

        /*
         * ASSIGNMENT
         */

        case AST_ASSIGNMENT:

            free(
                node->data.assignment.name
            );

            ast_free(
                node->data.assignment.value
            );

            break;

        /*
         * PRINT
         */

        case AST_PRINT:

            ast_free(
                node->data.print
            );

            break;

        /*
         * IF
         */

        case AST_IF:

            ast_free(
                node->data.if_statement.condition
            );

            ast_free(
                node->data.if_statement.then_branch
            );

            ast_free(
                node->data.if_statement.else_branch
            );

            break;

        /*
         * WHILE
         */

        case AST_WHILE:

            ast_free(
                node->data.while_statement.condition
            );

            ast_free(
                node->data.while_statement.body
            );

            break;

        /*
         * NUMBER
         * DECIMAL
         * BOOLEAN
         * NULL
         *
         * No necesitan free().
         */

        case AST_NUMBER:
        case AST_DECIMAL:
        case AST_BOOLEAN:
        case AST_NULL:

            break;

        /*
         * UNKNOWN
         */

        default:

            break;
    }

    free(node);
}
