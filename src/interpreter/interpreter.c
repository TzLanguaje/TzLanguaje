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

Environment *environment_create(
    Environment *parent
) {

    Environment *environment =
        malloc(sizeof(Environment));

    if (environment == NULL) {
        return NULL;
    }

    environment->count = 0;
    environment->capacity = 8;

    /*
     * Solo guardamos la referencia:
     * el padre pertenece a quien lo
     * creó.
     */

    environment->parent = parent;

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
 * BUSCAR EN ESTE SCOPE
 * ==========================
 *
 * NO sube por la cadena.
 */

static int environment_find_local(
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
 * BUSCAR EN LA CADENA
 * ==========================
 *
 * local → parent → ... → global
 *
 * Devuelve el scope que contiene
 * la variable, o NULL.
 */

static Environment *environment_owner(
    Environment *environment,
    const char *name
) {

    Environment *scope = environment;

    while (scope != NULL) {

        if (
            environment_find_local(
                scope,
                name
            ) >= 0
        ) {

            return scope;
        }

        scope = scope->parent;
    }

    return NULL;
}

/*
 * ==========================
 * ENTORNO GLOBAL
 * ==========================
 *
 * Sube hasta la raíz de la cadena.
 *
 * Es el entorno de definición de
 * TODAS las funciones de TzLang
 * hoy por hoy.
 */

static Environment *environment_global(
    Environment *environment
) {

    if (environment == NULL) {
        return NULL;
    }

    while (environment->parent != NULL) {
        environment = environment->parent;
    }

    return environment;
}

/*
 * ==========================
 * DECLARAR VARIABLE
 * ==========================
 *
 * variable x = 10
 *
 * Siempre en ESTE scope, nunca en
 * el padre: así una función puede
 * tapar una global sin tocarla.
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
        environment_find_local(
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

    /*
     * Buscamos subiendo por la
     * cadena de scopes.
     */

    Environment *scope =
        environment_owner(
            environment,
            name
        );

    if (scope == NULL) {
        return 0;
    }

    int index =
        environment_find_local(
            scope,
            name
        );

    /*
     * Devolvemos una copia.
     *
     * Esto es especialmente importante
     * para STRING porque contiene memoria
     * dinámica.
     */

    *value =
        value_copy(
            scope->variables[index].value
        );

    return 1;
}

/*
 * ==========================
 * ¿EXISTE LA VARIABLE?
 * ==========================
 */

int environment_has(
    Environment *environment,
    const char *name
) {

    if (environment == NULL) {
        return 0;
    }

    return
        environment_owner(
            environment,
            name
        ) != NULL;
}

/*
 * ==========================
 * ASIGNAR VARIABLE EXISTENTE
 * ==========================
 *
 * edad = 25
 *
 * Modifica la variable en el scope
 * donde exista: si es local, la
 * local; si viene del padre, la
 * del padre.
 */

int environment_assign(
    Environment *environment,
    const char *name,
    Value value
) {

    if (environment == NULL) {
        return 0;
    }

    Environment *scope =
        environment_owner(
            environment,
            name
        );

    /*
     * La asignación NO declara.
     */

    if (scope == NULL) {
        return 0;
    }

    int index =
        environment_find_local(
            scope,
            name
        );

    /*
     * Liberamos el valor anterior
     * antes de sustituirlo.
     */

    value_free(
        &scope->variables[index].value
    );

    scope->variables[index].value =
        value;

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

    /*
     * OJO: no se toca
     * environment->parent.
     *
     * El padre pertenece al contexto
     * que lo creó y sigue vivo.
     */

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
 * TABLA DE FUNCIONES
 * ==========================
 */

FunctionTable *function_table_create(void) {

    FunctionTable *table =
        malloc(sizeof(FunctionTable));

    if (table == NULL) {
        return NULL;
    }

    table->count = 0;
    table->capacity = 8;

    table->functions =
        malloc(
            sizeof(Function) *
            table->capacity
        );

    if (table->functions == NULL) {
        free(table);
        return NULL;
    }

    return table;
}

Function *function_table_find(
    FunctionTable *table,
    const char *name
) {

    if (table == NULL) {
        return NULL;
    }

    for (
        int i = 0;
        i < table->count;
        i++
    ) {

        if (
            strcmp(
                table->functions[i].name,
                name
            ) == 0
        ) {

            return &table->functions[i];
        }
    }

    return NULL;
}

int function_table_declare(
    FunctionTable *table,
    const char *name,
    ASTNode *body,
    Environment *closure
) {

    if (table == NULL) {
        return 0;
    }

    /*
     * ==========================
     * AUMENTAR CAPACIDAD
     * ==========================
     */

    if (table->count >= table->capacity) {

        int new_capacity =
            table->capacity * 2;

        Function *new_functions =
            realloc(
                table->functions,
                sizeof(Function) *
                new_capacity
            );

        if (new_functions == NULL) {
            return 0;
        }

        table->functions =
            new_functions;

        table->capacity =
            new_capacity;
    }

    char *stored_name =
        copy_string(name);

    if (stored_name == NULL) {
        return 0;
    }

    table->functions[table->count].name =
        stored_name;

    /*
     * Guardamos la referencia al
     * cuerpo, sin copiarlo.
     */

    table->functions[table->count].body =
        body;

    /*
     * Entorno de definición.
     *
     * Solo referencia: no lo
     * liberamos nunca aquí.
     */

    table->functions[table->count].closure =
        closure;

    table->count++;

    return 1;
}

void function_table_free(
    FunctionTable *table
) {

    if (table == NULL) {
        return;
    }

    for (
        int i = 0;
        i < table->count;
        i++
    ) {

        free(table->functions[i].name);

        /*
         * El cuerpo NO se libera
         * aquí: pertenece al AST.
         */
    }

    free(table->functions);

    free(table);
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
     * NULO
     * ==========================
     */

    if (node->type == AST_NULL) {

        *result = value_null();

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
     * UNARY
     * ==========================
     *
     * no activo
     */

    if (node->type == AST_UNARY) {

        Value operand;

        if (
            !evaluate_expression(
                node->data.unary.operand,
                environment,
                &operand
            )
        ) {

            return 0;
        }

        int success = 0;

        switch (node->data.unary.operator) {

            /*
             * NO
             */

            case OP_NOT:

                success =
                    operation_not(
                        operand,
                        result
                    );

                break;

            /*
             * OPERADOR DESCONOCIDO
             */

            default:

                fprintf(
                    stderr,
                    "Error: operador unario desconocido.\n"
                );

                success = 0;

                break;
        }

        value_free(&operand);

        return success;
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

            case OP_ADD:

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

            case OP_SUBTRACT:

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

            case OP_MULTIPLY:

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

            case OP_DIVIDE:

                success =
                    operation_divide(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * MAYOR QUE
             *
             * es mayor que
             */

            case OP_GREATER:

                success =
                    operation_greater(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * MENOR QUE
             *
             * es menor que
             */

            case OP_LESS:

                success =
                    operation_less(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * MAYOR O IGUAL QUE
             *
             * es mayor o igual que
             */

            case OP_GREATER_EQUAL:

                success =
                    operation_greater_equal(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * MENOR O IGUAL QUE
             *
             * es menor o igual que
             */

            case OP_LESS_EQUAL:

                success =
                    operation_less_equal(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * IGUAL A
             *
             * es igual a
             */

            case OP_EQUAL:

                success =
                    operation_equal(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * DIFERENTE DE
             *
             * es diferente de
             */

            case OP_NOT_EQUAL:

                success =
                    operation_not_equal(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * Y
             */

            case OP_AND:

                success =
                    operation_and(
                        left,
                        right,
                        result
                    );

                break;

            /*
             * O
             */

            case OP_OR:

                success =
                    operation_or(
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
                    "Error: operador binario desconocido.\n"
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

static int execute_block(
    ASTNode *block,
    Environment *environment,
    FunctionTable *functions
);

static int execute_statement(
    ASTNode *node,
    Environment *environment,
    FunctionTable *functions
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
     * ASIGNACIÓN
     * ==========================
     *
     * edad = 25
     */

    if (node->type == AST_ASSIGNMENT) {

        /*
         * 1. La variable debe existir.
         */

        if (
            !environment_has(
                environment,
                node->data.assignment.name
            )
        ) {

            fprintf(
                stderr,
                "Error: la variable '%s' no existe.\n",
                node->data.assignment.name
            );

            return 0;
        }

        /*
         * 2. Evaluar la expresión.
         */

        Value value;

        if (
            !evaluate_expression(
                node->data.assignment.value,
                environment,
                &value
            )
        ) {

            return 0;
        }

        /*
         * 3. Sustituir el valor
         *    anterior.
         */

        if (
            !environment_assign(
                environment,
                node->data.assignment.name,
                value
            )
        ) {

            value_free(&value);

            return 0;
        }

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
     * SI / SINO
     * ==========================
     *
     * si (condicion)
     *     ...
     * sino
     *     ...
     * fin
     */

    if (node->type == AST_IF) {

        Value condition;

        if (
            !evaluate_expression(
                node->data.if_statement.condition,
                environment,
                &condition
            )
        ) {

            return 0;
        }

        int is_true =
            value_is_truthy(condition);

        value_free(&condition);

        if (is_true) {

            return execute_block(
                node->data.if_statement.then_branch,
                environment,
                functions
            );
        }

        /*
         * El 'sino' es opcional.
         */

        if (
            node->data.if_statement.else_branch
            == NULL
        ) {

            return 1;
        }

        return execute_block(
            node->data.if_statement.else_branch,
            environment,
            functions
        );
    }

    /*
     * ==========================
     * MIENTRAS
     * ==========================
     *
     * mientras (condicion)
     *     ...
     * fin
     */

    if (node->type == AST_WHILE) {

        for (;;) {

            /*
             * 1. Evaluar la condición
             *    EN CADA VUELTA.
             */

            Value condition;

            if (
                !evaluate_expression(
                    node->data.while_statement.condition,
                    environment,
                    &condition
                )
            ) {

                return 0;
            }

            int is_true =
                value_is_truthy(condition);

            value_free(&condition);

            /*
             * 2. Si es falso, salimos.
             */

            if (!is_true) {
                return 1;
            }

            /*
             * 3. Ejecutar el cuerpo.
             *
             *    Si algo falla dentro,
             *    abortamos el bucle.
             */

            if (
                !execute_block(
                    node->data.while_statement.body,
                    environment,
                    functions
                )
            ) {

                return 0;
            }
        }
    }

    /*
     * ==========================
     * DECLARACIÓN DE FUNCIÓN
     * ==========================
     *
     * funcion saludar()
     *     ...
     * fin
     *
     * Solo REGISTRA. No ejecuta
     * el cuerpo.
     */

    if (node->type == AST_FUNCTION_DECLARATION) {

        const char *name =
            node->data.function_declaration.name;

        /*
         * No sobrescribimos en
         * silencio.
         */

        if (
            function_table_find(
                functions,
                name
            ) != NULL
        ) {

            fprintf(
                stderr,
                "Error: la función '%s' ya existe.\n",
                name
            );

            return 0;
        }

        /*
         * ==========================
         * ENTORNO DE DEFINICIÓN
         * ==========================
         *
         * En TzLang todas las
         * declaraciones de función
         * son globales, así que el
         * closure es el entorno
         * global.
         *
         * Guardamos la raíz de la
         * cadena y NO el entorno
         * actual a propósito: si una
         * declaración aparece dentro
         * de otra función, su entorno
         * local muere al terminar la
         * llamada y el closure
         * quedaría colgando.
         */

        if (
            !function_table_declare(
                functions,
                name,
                node->data.function_declaration.body,
                environment_global(environment)
            )
        ) {

            fprintf(
                stderr,
                "Error: no se pudo declarar la función '%s'.\n",
                name
            );

            return 0;
        }

        return 1;
    }

    /*
     * ==========================
     * LLAMADA A FUNCIÓN
     * ==========================
     *
     * saludar()
     */

    if (node->type == AST_FUNCTION_CALL) {

        const char *name =
            node->data.function_call.name;

        Function *function =
            function_table_find(
                functions,
                name
            );

        if (function == NULL) {

            fprintf(
                stderr,
                "Error: la función '%s' no existe.\n",
                name
            );

            return 0;
        }

        /*
         * ==========================
         * SCOPE LOCAL (LÉXICO)
         * ==========================
         *
         * El padre es el entorno donde
         * la función fue DEFINIDA, no
         * el de quien llama.
         *
         * Por eso una función solo ve
         * sus propias variables y las
         * de su ámbito de definición:
         * nunca las locales del
         * llamador.
         */

        Environment *local =
            environment_create(
                function->closure
            );

        if (local == NULL) {

            fprintf(
                stderr,
                "Error: no se pudo crear el entorno de '%s'.\n",
                name
            );

            return 0;
        }

        /*
         * El cuerpo es un bloque
         * como cualquier otro.
         */

        int success =
            execute_block(
                function->body,
                local,
                functions
            );

        /*
         * Destruir el scope local.
         *
         * Solo libera SUS variables:
         * el padre queda intacto.
         *
         * Se libera también cuando la
         * llamada falla, para no perder
         * memoria en la ruta de error.
         */

        environment_free(local);

        return success;
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
 * EJECUTAR BLOQUE
 * ==========================
 *
 * Lo usan el programa completo
 * y los cuerpos de 'si' y 'sino'.
 */

static int execute_block(
    ASTNode *block,
    Environment *environment,
    FunctionTable *functions
) {

    if (
        block == NULL ||
        block->type != AST_PROGRAM
    ) {

        return 0;
    }

    for (
        int i = 0;
        i < block->data.program.count;
        i++
    ) {

        if (
            !execute_statement(
                block->data.program.statements[i],
                environment,
                functions
            )
        ) {

            return 0;
        }
    }

    return 1;
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
     * Crear entorno GLOBAL
     *
     * Sin padre: es la raíz de la
     * cadena de scopes.
     */

    Environment *environment =
        environment_create(NULL);

    if (environment == NULL) {

        fprintf(
            stderr,
            "Error: no se pudo crear el entorno.\n"
        );

        return 0;
    }

    /*
     * ==========================
     * TABLA DE FUNCIONES
     * ==========================
     */

    FunctionTable *functions =
        function_table_create();

    if (functions == NULL) {

        fprintf(
            stderr,
            "Error: no se pudo crear la tabla de funciones.\n"
        );

        environment_free(environment);

        return 0;
    }

    /*
     * ==========================
     * EJECUTAR STATEMENTS
     * ==========================
     */

    if (
        !execute_block(
            program,
            environment,
            functions
        )
    ) {

        function_table_free(functions);

        environment_free(
            environment
        );

        return 0;
    }

    /*
     * ==========================
     * LIBERAR ENTORNO
     * ==========================
     */

    function_table_free(functions);

    environment_free(
        environment
    );

    return 1;
}