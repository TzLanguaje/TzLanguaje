#include "interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Copiar string
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
 * Crear Environment
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
 * Liberar Environment
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
    }

    free(environment->variables);
    free(environment);
}

/*
 * Buscar índice de variable
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
 * Guardar variable
 */
void environment_set(
    Environment *environment,
    const char *name,
    int value
) {

    if (environment == NULL) {
        return;
    }

    /*
     * ¿Ya existe?
     */
    int index =
        environment_find(
            environment,
            name
        );

    if (index >= 0) {

        environment->variables[index].value =
            value;

        return;
    }

    /*
     * ¿Necesitamos más espacio?
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
     * Crear nueva variable
     */
    environment->variables[
        environment->count
    ].name =
        copy_string(name);

    environment->variables[
        environment->count
    ].value =
        value;

    environment->count++;
}

/*
 * Obtener variable
 */
int environment_get(
    Environment *environment,
    const char *name,
    int *value
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

    *value =
        environment->variables[index].value;

    return 1;
}

/*
 * Evaluar expresión
 */
static int evaluate_expression(
    ASTNode *node,
    Environment *environment,
    int *result
) {

    if (
        node == NULL ||
        result == NULL
    ) {
        return 0;
    }

    /*
     * NUMBER
     *
     * 10
     */
    if (node->type == AST_NUMBER) {

        *result =
            node->data.number;

        return 1;
    }

    /*
     * IDENTIFIER
     *
     * x
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
     * BINARY
     *
     * x + y
     */
    if (node->type == AST_BINARY) {

        int left;
        int right;

        if (
            !evaluate_expression(
                node->data.binary.left,
                environment,
                &left
            )
        ) {
            return 0;
        }

        if (
            !evaluate_expression(
                node->data.binary.right,
                environment,
                &right
            )
        ) {
            return 0;
        }

        switch (
            node->data.binary.operator
        ) {

            case '+':
                *result = left + right;
                return 1;

            case '-':
                *result = left - right;
                return 1;

            case '*':
                *result = left * right;
                return 1;

            case '/':

                if (right == 0) {

                    fprintf(
                        stderr,
                        "Error: división por cero.\n"
                    );

                    return 0;
                }

                *result = left / right;

                return 1;

            default:

                fprintf(
                    stderr,
                    "Error: operador desconocido '%c'.\n",
                    node->data.binary.operator
                );

                return 0;
        }
    }

    fprintf(
        stderr,
        "Error: expresión desconocida.\n"
    );

    return 0;
}

/*
 * Ejecutar una instrucción
 */
static int execute_statement(
    ASTNode *node,
    Environment *environment
) {

    if (node == NULL) {
        return 0;
    }

    /*
     * VARIABLE
     *
     * variable x = 10;
     */
    if (
        node->type ==
        AST_VARIABLE_DECLARATION
    ) {

        int value;

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
     * PRINT
     *
     * imprimir(x);
     */
    if (node->type == AST_PRINT) {

        int value;

        if (
            !evaluate_expression(
                node->data.print,
                environment,
                &value
            )
        ) {
            return 0;
        }

        printf(
            "%d\n",
            value
        );

        return 1;
    }

    fprintf(
        stderr,
        "Error: instrucción desconocida.\n"
    );

    return 0;
}

/*
 * Ejecutar programa
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

    Environment *environment =
        environment_create();

    if (environment == NULL) {

        fprintf(
            stderr,
            "Error: no se pudo crear el entorno.\n"
        );

        return 0;
    }

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

    environment_free(
        environment
    );

    return 1;
}