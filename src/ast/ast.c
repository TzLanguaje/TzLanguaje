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

ASTNode *ast_return(ASTNode *value) {
    ASTNode *node = create_node(AST_RETURN);

    if (node == NULL) {
        return NULL;
    }

    node->data.return_value = value;

    return node;
}

ASTNode *ast_break(void) {
    return create_node(AST_BREAK);
}

ASTNode *ast_continue(void) {
    return create_node(AST_CONTINUE);
}

ASTNode *ast_list(
    ASTNode **elements,
    int element_count
) {
    ASTNode *node = create_node(AST_LIST);

    if (node == NULL) {
        return NULL;
    }

    node->data.list.elements = elements;
    node->data.list.element_count =
        element_count;

    return node;
}

ASTNode *ast_index(
    ASTNode *object,
    ASTNode *index
) {
    ASTNode *node = create_node(AST_INDEX);

    if (node == NULL) {
        return NULL;
    }

    node->data.index.object = object;
    node->data.index.index = index;

    return node;
}

ASTNode *ast_index_assignment(
    ASTNode *target,
    ASTNode *value
) {
    ASTNode *node =
        create_node(AST_INDEX_ASSIGNMENT);

    if (node == NULL) {
        return NULL;
    }

    node->data.index_assignment.target =
        target;

    node->data.index_assignment.value =
        value;

    return node;
}

ASTNode *ast_for_each(
    const char *variable,
    ASTNode *iterable,
    ASTNode *body
) {
    ASTNode *node = create_node(AST_FOR_EACH);

    if (node == NULL) {
        return NULL;
    }

    node->data.for_each.variable =
        copy_string(variable);

    if (node->data.for_each.variable == NULL) {
        free(node);
        return NULL;
    }

    node->data.for_each.iterable = iterable;
    node->data.for_each.body = body;

    return node;
}

ASTNode *ast_dictionary(
    ASTNode **keys,
    ASTNode **values,
    int pair_count
) {
    ASTNode *node = create_node(AST_DICTIONARY);

    if (node == NULL) {
        return NULL;
    }

    node->data.dictionary.keys = keys;
    node->data.dictionary.values = values;
    node->data.dictionary.pair_count = pair_count;

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

ASTNode *ast_function_declaration(
    const char *name,
    char **parameters,
    int parameter_count,
    ASTNode *body
) {
    ASTNode *node =
        create_node(AST_FUNCTION_DECLARATION);

    if (node == NULL) {
        return NULL;
    }

    node->data.function_declaration.name =
        copy_string(name);

    if (
        node->data.function_declaration.name
        == NULL
    ) {
        free(node);
        return NULL;
    }

    node->data.function_declaration.parameters =
        parameters;

    node->data.function_declaration.parameter_count =
        parameter_count;

    node->data.function_declaration.body =
        body;

    return node;
}

ASTNode *ast_function_call(
    const char *name,
    ASTNode **arguments,
    int argument_count
) {
    ASTNode *node =
        create_node(AST_FUNCTION_CALL);

    if (node == NULL) {
        return NULL;
    }

    node->data.function_call.name =
        copy_string(name);

    if (
        node->data.function_call.name
        == NULL
    ) {
        free(node);
        return NULL;
    }

    node->data.function_call.arguments =
        arguments;

    node->data.function_call.argument_count =
        argument_count;

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

        case OP_NEGATE:
            return "-";

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
         * RETURN
         */

        case AST_RETURN:

            if (node->data.return_value == NULL) {

                printf("RETURN: nulo\n");

                break;
            }

            printf("RETURN\n");

            ast_print_tree(
                node->data.return_value,
                indentation + 1
            );

            break;

        /*
         * BREAK / CONTINUE
         */

        case AST_BREAK:

            printf("BREAK\n");

            break;

        case AST_CONTINUE:

            printf("CONTINUE\n");

            break;

        /*
         * LIST
         */

        case AST_LIST:

            printf(
                "LIST (%d)\n",
                node->data.list.element_count
            );

            for (
                int i = 0;
                i < node->data.list.element_count;
                i++
            ) {
                ast_print_tree(
                    node->data.list.elements[i],
                    indentation + 1
                );
            }

            break;

        /*
         * DICTIONARY
         */

        case AST_DICTIONARY:

            printf(
                "DICTIONARY (%d)\n",
                node->data.dictionary.pair_count
            );

            for (
                int i = 0;
                i < node->data.dictionary.pair_count;
                i++
            ) {

                print_indent(indentation + 1);
                printf("PAIR\n");

                ast_print_tree(
                    node->data.dictionary.keys[i],
                    indentation + 2
                );

                ast_print_tree(
                    node->data.dictionary.values[i],
                    indentation + 2
                );
            }

            break;

        /*
         * INDEX
         */

        case AST_INDEX:

            printf("INDEX\n");

            ast_print_tree(
                node->data.index.object,
                indentation + 1
            );

            ast_print_tree(
                node->data.index.index,
                indentation + 1
            );

            break;

        /*
         * INDEX ASSIGNMENT
         */

        case AST_INDEX_ASSIGNMENT:

            printf("INDEX_ASSIGNMENT\n");

            ast_print_tree(
                node->data.index_assignment.target,
                indentation + 1
            );

            ast_print_tree(
                node->data.index_assignment.value,
                indentation + 1
            );

            break;

        /*
         * FOR EACH
         */

        case AST_FOR_EACH:

            printf(
                "FOR_EACH: %s\n",
                node->data.for_each.variable
            );

            print_indent(indentation + 1);
            printf("IN\n");

            ast_print_tree(
                node->data.for_each.iterable,
                indentation + 2
            );

            print_indent(indentation + 1);
            printf("BODY\n");

            ast_print_tree(
                node->data.for_each.body,
                indentation + 2
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
         * FUNCTION DECLARATION
         */

        case AST_FUNCTION_DECLARATION:

            printf(
                "FUNCTION: %s\n",
                node->data.function_declaration.name
            );

            print_indent(indentation + 1);
            printf("PARAMS (%d)",
                node->data.function_declaration.parameter_count
            );

            for (
                int i = 0;
                i < node->data.function_declaration.parameter_count;
                i++
            ) {
                printf(
                    "%s %s",
                    i == 0 ? ":" : ",",
                    node->data.function_declaration.parameters[i]
                );
            }

            printf("\n");

            print_indent(indentation + 1);
            printf("BODY\n");

            ast_print_tree(
                node->data.function_declaration.body,
                indentation + 2
            );

            break;

        /*
         * FUNCTION CALL
         */

        case AST_FUNCTION_CALL:

            printf(
                "CALL: %s (%d args)\n",
                node->data.function_call.name,
                node->data.function_call.argument_count
            );

            for (
                int i = 0;
                i < node->data.function_call.argument_count;
                i++
            ) {
                ast_print_tree(
                    node->data.function_call.arguments[i],
                    indentation + 1
                );
            }

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
         * RETURN
         *
         * return_value puede ser NULL
         * y ast_free lo tolera.
         */

        case AST_RETURN:

            ast_free(
                node->data.return_value
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
         * FUNCTION DECLARATION
         *
         * El AST es el dueño del
         * cuerpo. La tabla de
         * funciones solo lo apunta.
         */

        case AST_FUNCTION_DECLARATION:

            free(
                node->data.function_declaration.name
            );

            /*
             * Los nombres de los
             * parámetros son nuestros.
             */

            for (
                int i = 0;
                i < node->data.function_declaration.parameter_count;
                i++
            ) {
                free(
                    node->data.function_declaration.parameters[i]
                );
            }

            free(
                node->data.function_declaration.parameters
            );

            ast_free(
                node->data.function_declaration.body
            );

            break;

        /*
         * FUNCTION CALL
         */

        case AST_FUNCTION_CALL:

            free(
                node->data.function_call.name
            );

            /*
             * Los argumentos son
             * expresiones nuestras.
             */

            for (
                int i = 0;
                i < node->data.function_call.argument_count;
                i++
            ) {
                ast_free(
                    node->data.function_call.arguments[i]
                );
            }

            free(
                node->data.function_call.arguments
            );

            break;

        /*
         * LIST
         */

        case AST_LIST:

            for (
                int i = 0;
                i < node->data.list.element_count;
                i++
            ) {
                ast_free(
                    node->data.list.elements[i]
                );
            }

            free(node->data.list.elements);

            break;

        /*
         * DICTIONARY
         */

        case AST_DICTIONARY:

            for (
                int i = 0;
                i < node->data.dictionary.pair_count;
                i++
            ) {
                ast_free(
                    node->data.dictionary.keys[i]
                );

                ast_free(
                    node->data.dictionary.values[i]
                );
            }

            free(node->data.dictionary.keys);
            free(node->data.dictionary.values);

            break;

        /*
         * INDEX
         */

        case AST_INDEX:

            ast_free(
                node->data.index.object
            );

            ast_free(
                node->data.index.index
            );

            break;

        /*
         * INDEX ASSIGNMENT
         */

        case AST_INDEX_ASSIGNMENT:

            ast_free(
                node->data.index_assignment.target
            );

            ast_free(
                node->data.index_assignment.value
            );

            break;

        /*
         * FOR EACH
         */

        case AST_FOR_EACH:

            free(
                node->data.for_each.variable
            );

            ast_free(
                node->data.for_each.iterable
            );

            ast_free(
                node->data.for_each.body
            );

            break;

        /*
         * NUMBER
         * DECIMAL
         * BOOLEAN
         * NULL
         * BREAK
         * CONTINUE
         *
         * No necesitan free().
         */

        case AST_NUMBER:
        case AST_DECIMAL:
        case AST_BOOLEAN:
        case AST_NULL:
        case AST_BREAK:
        case AST_CONTINUE:

            break;

        /*
         * UNKNOWN
         */

        default:

            break;
    }

    free(node);
}
