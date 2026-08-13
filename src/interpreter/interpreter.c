#include "interpreter.h"
#include "../runtime/operations.h"

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

    char *result =
        malloc(length + 1);

    if (result == NULL) {
        return NULL;
    }

    strcpy(result, source);

    return result;
}

/*
 * ==========================
 * COPIAR VALUE
 * ==========================
 */

static Value value_copy(Value source) {

    switch (source.type) {

        case VALUE_NUMBER:

            return value_number(
                source.data.number
            );

        case VALUE_DECIMAL:

            return value_decimal(
                source.data.decimal
            );

        case VALUE_STRING:

            return value_string(
                source.data.string
            );

        case VALUE_BOOLEAN:

            return value_boolean(
                source.data.boolean
            );

        case VALUE_NULL:

            return value_null();

        default:

            return value_null();
    }
}

/*
 * ==========================
 * ENVIRONMENT
 * ==========================
 */

Environment *environment_create(void) {

    Environment *environment =
        malloc(sizeof(Environment));

    if (environment == NULL) {
        return NULL;
    }

    environment->count = 0;
    environment->capacity = 8;

    environment->variables =
        malloc(
            sizeof(Variable) *
            environment->capacity
        );

    if (environment->variables == NULL) {
        free(environment);
        return NULL;
    }

    return environment;
}

/*
 * ==========================
 * BUSCAR VARIABLE
 * ==========================
 */

static int environment_find(
    Environment *environment,
    const char *name
) {

    for (
        int i = 0;
        i < environment->count;
        i++
    ) {

        if (
            strcmp(
                environment->variables[i].name,
                name
            ) == 0
        ) {

            return i;
        }
    }

    return -1;
}

/*
 * ==========================
 * GUARDAR VARIABLE
 * ==========================
 */

void environment_set(
    Environment *environment,
    const char *name,
    Value value
) {

    if (environment == NULL) {
        return;
    }

    int index =
        environment_find(
            environment,
            name
        );

    /*
     * ==========================
     * VARIABLE YA EXISTENTE
     * ==========================
     */

    if (index >= 0) {

        value_free(
            &environment->variables[index].value
        );

        environment->variables[index].value =
            value;

        return;
    }

    /*
     * ==========================
     * AUMENTAR CAPACIDAD
     * ==========================
     */

    if (
        environment->count >=
        environment->capacity
    ) {

        int new_capacity =
            environment->capacity * 2;

        Variable *new_variables =
            realloc(
                environment->variables,
                sizeof(Variable) *
                new_capacity
            );

        if (new_variables == NULL) {
            return;
        }

        environment->variables =
            new_variables;

        environment->capacity =
            new_capacity;
    }

    /*
     * ==========================
     * GUARDAR NOMBRE
     * ==========================
     */

    environment->variables[
        environment->count
    ].name =
        copy_string(name);

    /*
     * ==========================
     * GUARDAR VALUE
     * ==========================
     */

    environment->variables[
        environment->count
    ].value =
        value;

    environment->count++;
}

/*
 * ==========================
 * OBTENER VARIABLE
 * ==========================
 */

int environment_get(
    Environment *environment,
    const char *name,
    Value *value
) {

    if (
        environment == NULL ||
        value == NULL
    ) {

        return 0;
    }

    int index =
        environment_find(
            environment,
            name
        );

    if (index < 0) {
        return 0;
    }

    /*
     * Devolvemos una copia.
     *
     * Esto es especialmente importante
     * para STRING porque contiene memoria
     * dinámica.
     */

    *value =
        value_copy(
            environment->variables[index].value
        );

    return 1;
}

/*
 * ==========================
 * LIBERAR ENVIRONMENT
 * ==========================
 */

void environment_free(
    Environment *environment
) {

    if (environment == NULL) {
        return;
    }

    for (
        int i = 0;
        i < environment->count;
        i++
    ) {

        free(
            environment->variables[i].name
        );

        value_free(
            &environment->variables[i].value
        );
    }

    free(
        environment->variables
    );

    free(environment);
}

/*
 * ==========================
 * EVALUAR EXPRESIONES
 * ==========================
 */

static int evaluate_expression(
    ASTNode *node,
    Environment *environment,
    Value *result
) {

    if (
        node == NULL ||
        result == NULL
    ) {

        return 0;
    }

    /*
     * ==========================
     * NUMBER
     * ==========================
     */

    if (node->type == AST_NUMBER) {

        *result =
            value_number(
                node->data.number
            );

        return 1;
    }

    /*
     * ==========================
     * DECIMAL
     * ==========================
     */

    if (node->type == AST_DECIMAL) {

        *result =
            value_decimal(
                node->data.decimal
            );

        return 1;
    }

    /*
     * ==========================
     * STRING
     * ==========================
     */

    if (node->type == AST_STRING) {

        *result =
            value_string(
                node->data.string
            );

        return 1;
    }

    /*
     * ==========================
     * BOOLEAN
     * ==========================
     */

    if (node->type == AST_BOOLEAN) {

        *result =
            value_boolean(
                node->data.boolean
            );

        return 1;
    }

    /*
     * ==========================
     * IDENTIFIER
     * ==========================
     */

    if (node->type == AST_IDENTIFIER) {

        if (
            !environment_get(
                environment,
                node->data.identifier,
                result
            )
        ) {

            fprintf(
                stderr,
                "Error: variable '%s' no existe.\n",
                node->data.identifier
            );

            return 0;
        }

        return 1;
    }

    /*
     * ==========================
     * BINARY
     * ==========================
     */

    if (node->type == AST_BINARY) {

        Value left;
        Value right;

        /*
         * Evaluar izquierda
         */

        if (
            !evaluate_expression(
                node->data.binary.left,
                environment,
                &left
            )
        ) {

            return 0;
        }

        /*
         * Evaluar derecha
         */

        if (
            !evaluate_expression(
                node->data.binary.right,
                environment,
                &right
            )
        ) {

            value_free(&left);

            return 0;
        }

        int success = 0;

        /*
         * ==========================
         * EJECUTAR OPERACIÓN
         * ==========================
         */

        switch (
            node->data.binary.operator
        ) {

            /*
             * SUMA
             */

            case '+':

                success =
                    operation_add(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * RESTA
             */

            case '-':

                success =
                    operation_subtract(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * MULTIPLICACIÓN
             */

            case '*':

                success =
                    operation_multiply(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * DIVISIÓN
             */

            case '/':

                success =
                    operation_divide(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * OPERADOR DESCONOCIDO
             */

            default:

                fprintf(
                    stderr,
                    "Error: operador desconocido '%c'.\n",
                    node->data.binary.operator
                );

                success = 0;

                break;
        }

        /*
         * ==========================
         * LIBERAR OPERANDOS
         * ==========================
         */

        value_free(&left);

        value_free(&right);

        return success;
    }

    /*
     * ==========================
     * EXPRESIÓN DESCONOCIDA
     * ==========================
     */

    fprintf(
        stderr,
        "Error: expresión desconocida.\n"
    );

    return 0;
}

/*
 * ==========================
 * EJECUTAR STATEMENT
 * ==========================
 */

static int execute_statement(
    ASTNode *node,
    Environment *environment
) {

    if (node == NULL) {
        return 0;
    }

    /*
     * ==========================
     * VARIABLE
     * ==========================
     *
     * variable x = ...
     */

    if (
        node->type ==
        AST_VARIABLE_DECLARATION
    ) {

        Value value;

        if (
            !evaluate_expression(
                node->data.variable.value,
                environment,
                &value
            )
        ) {

            return 0;
        }

        environment_set(
            environment,
            node->data.variable.name,
            value
        );

        return 1;
    }

    /*
     * ==========================
     * PRINT
     * ==========================
     *
     * imprimir(...)
     */

    if (node->type == AST_PRINT) {

        Value value;

        if (
            !evaluate_expression(
                node->data.print,
                environment,
                &value
            )
        ) {

            return 0;
        }

        value_print(value);

        printf("\n");

        value_free(&value);

        return 1;
    }

    /*
     * ==========================
     * STATEMENT DESCONOCIDO
     * ==========================
     */

    fprintf(
        stderr,
        "Error: instrucción desconocida.\n"
    );

    return 0;
}

/*
 * ==========================
 * EJECUTAR PROGRAMA
 * ==========================
 */

int interpreter_run(
    ASTNode *program
) {

    if (
        program == NULL ||
        program->type != AST_PROGRAM
    ) {

        return 0;
    }

    /*
     * Crear entorno
     */

    Environment *environment =
        environment_create();

    if (environment == NULL) {

        fprintf(
            stderr,
            "Error: no se pudo crear el entorno.\n"
        );

        return 0;
    }

    /*
     * ==========================
     * EJECUTAR STATEMENTS
     * ==========================
     */

    for (
        int i = 0;
        i < program->data.program.count;
        i++
    ) {

        if (
            !execute_statement(
                program->data.program.statements[i],
                environment
            )
        ) {

            environment_free(
                environment
            );

            return 0;
        }
    }

    /*
     * ==========================
     * LIBERAR ENTORNO
     * ==========================
     */

    environment_free(
        environment
    );

    return 1;
}