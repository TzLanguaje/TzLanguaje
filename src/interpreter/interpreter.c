#include "interpreter.h"

#include "../diagnostic/diagnostic.h"
#include "../runtime/operations.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
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
 * REFERENCIA A LA VARIABLE
 * ==========================
 *
 * Sin copiar: apunta al Value
 * real, para poder modificarlo en
 * el sitio.
 */

Value *environment_get_ref(
    Environment *environment,
    const char *name
) {

    if (environment == NULL) {
        return NULL;
    }

    Environment *scope =
        environment_owner(
            environment,
            name
        );

    if (scope == NULL) {
        return NULL;
    }

    int index =
        environment_find_local(
            scope,
            name
        );

    return &scope->variables[index].value;
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
    char **parameters,
    int parameter_count,
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
     * Los parámetros también son
     * del AST: solo apuntamos.
     */

    table->functions[table->count].parameters =
        parameters;

    table->functions[table->count].parameter_count =
        parameter_count;

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
 * RESULTADO DE EJECUCIÓN
 * ==========================
 *
 * Así viaja un 'retornar' desde
 * donde aparece hasta la llamada
 * que lo provocó:
 *
 * AST_RETURN
 *      ↓  returned = 1
 * execute_block   (corta el bloque)
 *      ↓
 * execute_block   (corta el de fuera)
 *      ↓
 * call_function   (recoge el Value)
 *      ↓
 * evaluate_expression
 *
 * Sin variables globales y sin
 * setjmp: el estado va y vuelve
 * por la pila de llamadas de C.
 *
 * QUIÉN CONSUME QUÉ
 *
 * Esta es la diferencia clave
 * entre las tres señales:
 *
 * returned  → NADIE lo consume
 *             salvo la llamada:
 *             atraviesa 'si' y
 *             'mientras' hasta
 *             salir de la función.
 *
 * broke     → lo consume el
 *             'mientras' más
 *             cercano, que termina.
 *             No sale de él.
 *
 * continued → lo consume el
 *             'mientras' más
 *             cercano, que pasa a
 *             la siguiente vuelta.
 *             No sale de él.
 *
 * Por eso en bucles anidados
 * 'romper' solo afecta al interno:
 * el interno lo consume y nunca
 * llega al externo.
 *
 * PROPIEDAD DE 'value':
 *
 * Cuando returned es 1, el Value
 * pasa a ser de quien recibe el
 * ExecutionResult, que debe
 * liberarlo o cederlo.
 *
 * En cualquier otro caso —incluidos
 * broke y continued— value es
 * siempre nulo, así que liberarlo
 * es inofensivo.
 */

typedef struct {
    int success;
    int returned;
    int broke;
    int continued;
    Value value;
} ExecutionResult;

static ExecutionResult execution_ok(void) {

    ExecutionResult result;

    result.success = 1;
    result.returned = 0;
    result.broke = 0;
    result.continued = 0;
    result.value = value_null();

    return result;
}

static ExecutionResult execution_error(void) {

    ExecutionResult result = execution_ok();

    result.success = 0;

    return result;
}

static ExecutionResult execution_return(
    Value value
) {

    ExecutionResult result = execution_ok();

    result.returned = 1;
    result.value = value;

    return result;
}

static ExecutionResult execution_break(void) {

    ExecutionResult result = execution_ok();

    result.broke = 1;

    return result;
}

static ExecutionResult execution_continue(void) {

    ExecutionResult result = execution_ok();

    result.continued = 1;

    return result;
}

/*
 * ¿Este resultado corta el bloque
 * en curso?
 *
 * Lo hacen los cuatro casos: error,
 * retorno, romper y continuar. La
 * diferencia está en QUIÉN los
 * consume después.
 */

static int execution_interrupts(
    ExecutionResult result
) {

    return
        !result.success ||
        result.returned ||
        result.broke ||
        result.continued;
}

/*
 * ==========================
 * EVALUAR EXPRESIONES
 * ==========================
 */

/*
 * Ejecuta una llamada y produce
 * su valor.
 *
 * La comparten la llamada como
 * instrucción y la llamada como
 * expresión, así que no hay dos
 * implementaciones de lo mismo.
 */
static int call_function(
    ASTNode *node,
    Environment *environment,
    FunctionTable *functions,
    Value *result
);

static int evaluate_expression(
    ASTNode *node,
    Environment *environment,
    FunctionTable *functions,
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

            diagnostic_registrar(DIAG_VARIABLE_NO_DEFINIDA);

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
                functions,
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
             * MENOS UNARIO
             */

            case OP_NEGATE:

                success =
                    operation_negate(
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
                functions,
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
                functions,
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
     * LITERAL DE LISTA
     * ==========================
     *
     * [10, 20, 30]
     *
     * Cada elemento es una expresión
     * que se evalúa AHORA, no al
     * analizar.
     */

    if (node->type == AST_LIST) {

        Value list = value_list();

        if (list.data.list == NULL) {

            fprintf(
                stderr,
                "Error: no se pudo crear la lista.\n"
            );

            return 0;
        }

        for (
            int i = 0;
            i < node->data.list.element_count;
            i++
        ) {

            Value item;

            if (
                !evaluate_expression(
                    node->data.list.elements[i],
                    environment,
                    functions,
                    &item
                )
            ) {

                /*
                 * Se libera la lista a
                 * medio construir, con
                 * todo lo que ya lleve
                 * dentro.
                 */

                value_free(&list);

                return 0;
            }

            /*
             * La lista se queda con el
             * item.
             */

            if (!value_list_push(&list, item)) {

                fprintf(
                    stderr,
                    "Error: no se pudo ampliar la lista.\n"
                );

                value_free(&item);
                value_free(&list);

                return 0;
            }
        }

        *result = list;

        return 1;
    }

    /*
     * ==========================
     * LITERAL DE DICCIONARIO
     * ==========================
     *
     * {"nombre": "Carlos"}
     *
     * Claves y valores se evaluan
     * AHORA. La clave debe dar un
     * texto.
     */

    if (node->type == AST_DICTIONARY) {

        Value dictionary = value_dictionary();

        if (dictionary.data.dictionary == NULL) {

            fprintf(
                stderr,
                "Error: no se pudo crear el diccionario.\n"
            );

            return 0;
        }

        for (
            int i = 0;
            i < node->data.dictionary.pair_count;
            i++
        ) {

            Value key;

            if (
                !evaluate_expression(
                    node->data.dictionary.keys[i],
                    environment,
                    functions,
                    &key
                )
            ) {

                value_free(&dictionary);

                return 0;
            }

            if (key.type != VALUE_STRING) {

                diagnostic_registrar(DIAG_TIPO);

                fprintf(
                    stderr,
                    "Error: la clave de un diccionario debe ser un texto, no %s.\n",
                    value_type_name(key.type)
                );

                value_free(&key);
                value_free(&dictionary);

                return 0;
            }

            Value item;

            if (
                !evaluate_expression(
                    node->data.dictionary.values[i],
                    environment,
                    functions,
                    &item
                )
            ) {

                value_free(&key);
                value_free(&dictionary);

                return 0;
            }

            /*
             * set copia la clave y se
             * queda con el valor.
             */

            if (
                !value_dictionary_set(
                    &dictionary,
                    key.data.string,
                    item
                )
            ) {

                fprintf(
                    stderr,
                    "Error: no se pudo ampliar el diccionario.\n"
                );

                value_free(&item);
                value_free(&key);
                value_free(&dictionary);

                return 0;
            }

            value_free(&key);
        }

        *result = dictionary;

        return 1;
    }

    /*
     * ==========================
     * INDEXACIÓN
     * ==========================
     *
     * numeros[0]         (lista)
     * persona["nombre"]  (diccionario)
     * usuarios[0]["edad"]
     *
     * Una sola implementacion: el
     * tipo del contenedor decide en
     * ejecucion, no la gramatica.
     */

    if (node->type == AST_INDEX) {

        Value object;

        if (
            !evaluate_expression(
                node->data.index.object,
                environment,
                functions,
                &object
            )
        ) {

            return 0;
        }

        if (
            object.type != VALUE_LIST &&
            object.type != VALUE_DICTIONARY
        ) {

            diagnostic_registrar(DIAG_TIPO);

            fprintf(
                stderr,
                "Error: solo se puede indexar una lista o un diccionario, no %s.\n",
                value_type_name(object.type)
            );

            value_free(&object);

            return 0;
        }

        Value index;

        if (
            !evaluate_expression(
                node->data.index.index,
                environment,
                functions,
                &index
            )
        ) {

            value_free(&object);

            return 0;
        }

        /*
         * LISTA: indice numerico
         */

        if (object.type == VALUE_LIST) {

            if (index.type != VALUE_NUMBER) {

                diagnostic_registrar(DIAG_TIPO);

                fprintf(
                    stderr,
                    "Error: el índice debe ser un numero, no %s.\n",
                    value_type_name(index.type)
                );

                value_free(&object);
                value_free(&index);

                return 0;
            }

            int position = index.data.number;

            Value *item =
                value_list_at(object, position);

            if (item == NULL) {

                diagnostic_registrar(DIAG_INDICE);

                fprintf(
                    stderr,
                    "Error: índice %d fuera de rango. "
                    "La lista tiene %d elemento%s.\n",
                    position,
                    value_list_count(object),
                    value_list_count(object) == 1
                        ? ""
                        : "s"
                );

                value_free(&object);
                value_free(&index);

                return 0;
            }

            /*
             * Copia: el resultado no debe
             * depender de 'object', que se
             * libera aquí mismo.
             */

            *result = value_copy(*item);

            value_free(&object);
            value_free(&index);

            return 1;
        }

        /*
         * DICCIONARIO: clave de texto
         */

        if (index.type != VALUE_STRING) {

            diagnostic_registrar(DIAG_TIPO);

            fprintf(
                stderr,
                "Error: la clave debe ser un texto, no %s.\n",
                value_type_name(index.type)
            );

            value_free(&object);
            value_free(&index);

            return 0;
        }

        Value *item =
            value_dictionary_at(
                object,
                index.data.string
            );

        if (item == NULL) {

            diagnostic_registrar(DIAG_CLAVE);

            fprintf(
                stderr,
                "Error: la clave '%s' no existe en el diccionario.\n",
                index.data.string
            );

            value_free(&object);
            value_free(&index);

            return 0;
        }

        *result = value_copy(*item);

        value_free(&object);
        value_free(&index);

        return 1;
    }

    /*
     * ==========================
     * LLAMADA A FUNCIÓN
     * ==========================
     *
     * sumar(10, 20) * 2
     *
     * Una función sin 'retornar'
     * produce nulo, así que TODA
     * llamada es una expresión
     * válida.
     */

    if (node->type == AST_FUNCTION_CALL) {

        return call_function(
            node,
            environment,
            functions,
            result
        );
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

static ExecutionResult execute_block(
    ASTNode *block,
    Environment *environment,
    FunctionTable *functions
);

/*
 * ==========================
 * LLAMAR A UNA FUNCIÓN
 * ==========================
 *
 * Punto único: lo usan tanto la
 * llamada como instrucción como
 * la llamada dentro de una
 * expresión.
 */

/*
 * Definida mas abajo: resuelve una
 * posicion indexada a un puntero
 * interno.
 */
static int resolve_index_slot(
    ASTNode *node,
    Environment *environment,
    FunctionTable *functions,
    int create,
    Value **slot
);

/*
 * ==========================
 * BIBLIOTECA ESTANDAR
 * ==========================
 *
 * Funciones INCORPORADAS: no las
 * escribe el usuario y no viven en
 * la FunctionTable.
 *
 * Se resuelven en call_function
 * ANTES que las funciones del
 * usuario, y declarar una funcion
 * con uno de estos nombres es un
 * error, para que nunca queden
 * tapadas en silencio.
 *
 * Dos familias:
 *
 * PURAS      evaluan sus argumentos
 *            normalmente (por copia).
 *
 * MUTADORAS  'agregar' y 'eliminar'
 *            necesitan modificar la
 *            lista o el diccionario
 *            ORIGINAL. Como TzLang
 *            es de copia profunda,
 *            evaluar el argumento
 *            daria una copia y el
 *            cambio se perderia: por
 *            eso el primer argumento
 *            se resuelve POR
 *            REFERENCIA, reutilizando
 *            environment_get_ref y
 *            resolve_index_slot.
 */

typedef struct {
    const char *name;
    int arity;
    int mutates_first;
} Builtin;

static const Builtin builtins[] = {
    { "largo",    1, 0 },
    { "tipo",     1, 0 },
    { "texto",    1, 0 },
    { "numero",   1, 0 },
    { "decimal",  1, 0 },
    { "contiene", 2, 0 },
    { "agregar",  2, 1 },
    { "eliminar", 2, 1 },

    /*
     * Texto
     */

    { "unir",       2, 0 },
    { "separar",    2, 0 },
    { "mayusculas", 1, 0 },
    { "minusculas", 1, 0 },

    /*
     * Numeros
     */

    { "absoluto",   1, 0 },
    { "redondear",  1, 0 },

    /*
     * Diccionarios
     */

    { "claves",     1, 0 },
    { "valores",    1, 0 }
};

static const Builtin *builtin_find(
    const char *name
) {

    size_t count =
        sizeof(builtins) / sizeof(builtins[0]);

    for (size_t i = 0; i < count; i++) {

        if (strcmp(name, builtins[i].name) == 0) {
            return &builtins[i];
        }
    }

    return NULL;
}

/*
 * Resuelve un argumento que hay que
 * MODIFICAR, no leer.
 *
 * Vale una variable o una posicion
 * indexada:
 *
 * agregar(numeros, 4)
 * agregar(datos["lista"], 4)
 */

static int resolve_lvalue(
    ASTNode *node,
    Environment *environment,
    FunctionTable *functions,
    Value **slot
) {

    if (node->type == AST_IDENTIFIER) {

        Value *reference =
            environment_get_ref(
                environment,
                node->data.identifier
            );

        if (reference == NULL) {

            diagnostic_registrar(DIAG_VARIABLE_NO_DEFINIDA);

            fprintf(
                stderr,
                "Error: variable '%s' no existe.\n",
                node->data.identifier
            );

            return 0;
        }

        *slot = reference;

        return 1;
    }

    if (node->type == AST_INDEX) {

        return resolve_index_slot(
            node,
            environment,
            functions,
            0,
            slot
        );
    }

    diagnostic_registrar(DIAG_ARGUMENTO);

    fprintf(
        stderr,
        "Error: el primer argumento debe ser una variable.\n"
    );

    return 0;
}

/*
 * texto(valor)
 *
 * Solo valores basicos: una lista o
 * un diccionario no tienen una
 * representacion de texto util
 * todavia.
 */

static int builtin_texto(
    Value value,
    Value *result
) {

    char buffer[64];

    switch (value.type) {

        case VALUE_STRING:

            *result = value_copy(value);

            return 1;

        case VALUE_NUMBER:

            snprintf(
                buffer,
                sizeof(buffer),
                "%d",
                value.data.number
            );

            *result = value_string(buffer);

            return 1;

        case VALUE_DECIMAL:

            snprintf(
                buffer,
                sizeof(buffer),
                "%g",
                value.data.decimal
            );

            *result = value_string(buffer);

            return 1;

        case VALUE_BOOLEAN:

            *result =
                value_string(
                    value.data.boolean
                        ? "verdadero"
                        : "falso"
                );

            return 1;

        case VALUE_NULL:

            *result = value_string("nulo");

            return 1;

        default:

            diagnostic_registrar(DIAG_ARGUMENTO);

            fprintf(
                stderr,
                "Error: texto() no puede convertir %s.\n",
                value_type_name(value.type)
            );

            return 0;
    }
}

static int decimal_cabe_en_numero(double value);

/*
 * numero(valor) y decimal(valor)
 *
 * Desde texto se exige que TODA la
 * cadena sea el numero: "12abc" y
 * "1.5" no son enteros validos.
 */

static int builtin_numero(
    Value value,
    Value *result
) {

    if (value.type == VALUE_NUMBER) {
        *result = value_number(value.data.number);
        return 1;
    }

    if (value.type == VALUE_DECIMAL) {

        if (!decimal_cabe_en_numero(value.data.decimal)) {

            diagnostic_registrar(DIAG_ARGUMENTO);

            fprintf(
                stderr,
                "Error: numero() no puede convertir %g.\n",
                value.data.decimal
            );

            return 0;
        }

        *result =
            value_number((int)value.data.decimal);
        return 1;
    }

    if (value.type == VALUE_BOOLEAN) {
        *result =
            value_number(value.data.boolean);
        return 1;
    }

    if (value.type == VALUE_STRING) {

        const char *text = value.data.string;

        if (text == NULL || text[0] == '\0') {

            diagnostic_registrar(DIAG_ARGUMENTO);

            fprintf(
                stderr,
                "Error: numero() no puede convertir un texto vacío.\n"
            );

            return 0;
        }

        errno = 0;

        char *end = NULL;

        long long parsed = strtoll(text, &end, 10);

        if (end == text || *end != '\0') {

            diagnostic_registrar(DIAG_ARGUMENTO);

            fprintf(
                stderr,
                "Error: numero() no puede convertir el texto '%s'.\n",
                text
            );

            return 0;
        }

        /*
         * El texto puede ser un entero
         * valido pero demasiado grande
         * para un numero de TzLang.
         */

        if (
            errno == ERANGE ||
            parsed < (long long)INT_MIN ||
            parsed > (long long)INT_MAX
        ) {

            diagnostic_registrar(DIAG_ARGUMENTO);

            fprintf(
                stderr,
                "Error: el número %s está fuera del rango permitido "
                "(de -2147483648 a 2147483647).\n",
                text
            );

            return 0;
        }

        *result = value_number((int)parsed);

        return 1;
    }

    diagnostic_registrar(DIAG_ARGUMENTO);

    fprintf(
        stderr,
        "Error: numero() no puede convertir %s.\n",
        value_type_name(value.type)
    );

    return 0;
}

static int builtin_decimal(
    Value value,
    Value *result
) {

    if (value.type == VALUE_DECIMAL) {
        *result = value_decimal(value.data.decimal);
        return 1;
    }

    if (value.type == VALUE_NUMBER) {
        *result =
            value_decimal((double)value.data.number);
        return 1;
    }

    if (value.type == VALUE_BOOLEAN) {
        *result =
            value_decimal(
                value.data.boolean ? 1.0 : 0.0
            );
        return 1;
    }

    if (value.type == VALUE_STRING) {

        const char *text = value.data.string;

        if (text == NULL || text[0] == '\0') {

            diagnostic_registrar(DIAG_ARGUMENTO);

            fprintf(
                stderr,
                "Error: decimal() no puede convertir un texto vacío.\n"
            );

            return 0;
        }

        char *end = NULL;

        double parsed = strtod(text, &end);

        if (end == text || *end != '\0') {

            diagnostic_registrar(DIAG_ARGUMENTO);

            fprintf(
                stderr,
                "Error: decimal() no puede convertir el texto '%s'.\n",
                text
            );

            return 0;
        }

        *result = value_decimal(parsed);

        return 1;
    }

    diagnostic_registrar(DIAG_ARGUMENTO);

    fprintf(
        stderr,
        "Error: decimal() no puede convertir %s.\n",
        value_type_name(value.type)
    );

    return 0;
}

/*
 * largo(valor)
 *
 * texto        → numero de caracteres
 * lista        → numero de elementos
 * diccionario  → numero de claves
 */

static int builtin_largo(
    Value value,
    Value *result
) {

    if (value.type == VALUE_STRING) {

        *result =
            value_number(
                value.data.string == NULL
                    ? 0
                    : (int)strlen(value.data.string)
            );

        return 1;
    }

    if (value.type == VALUE_LIST) {

        *result =
            value_number(value_list_count(value));

        return 1;
    }

    if (value.type == VALUE_DICTIONARY) {

        *result =
            value_number(
                value_dictionary_count(value)
            );

        return 1;
    }

    diagnostic_registrar(DIAG_ARGUMENTO);

    fprintf(
        stderr,
        "Error: largo() solo funciona con texto, lista o diccionario, no %s.\n",
        value_type_name(value.type)
    );

    return 0;
}

/*
 * contiene(lista, valor)       → busca por VALOR
 * contiene(diccionario, clave) → busca la CLAVE
 */

static int builtin_contiene(
    Value container,
    Value needle,
    Value *result
) {

    if (container.type == VALUE_LIST) {

        int count = value_list_count(container);

        for (int i = 0; i < count; i++) {

            Value comparison;

            /*
             * Reutiliza la igualdad
             * estructural del runtime.
             */

            if (
                !operation_equal(
                    *value_list_at(container, i),
                    needle,
                    &comparison
                )
            ) {

                return 0;
            }

            int equal =
                value_is_truthy(comparison);

            value_free(&comparison);

            if (equal) {

                *result = value_boolean(1);

                return 1;
            }
        }

        *result = value_boolean(0);

        return 1;
    }

    if (container.type == VALUE_DICTIONARY) {

        if (needle.type != VALUE_STRING) {

            diagnostic_registrar(DIAG_TIPO);

            fprintf(
                stderr,
                "Error: la clave debe ser un texto, no %s.\n",
                value_type_name(needle.type)
            );

            return 0;
        }

        *result =
            value_boolean(
                value_dictionary_contains(
                    container,
                    needle.data.string
                )
            );

        return 1;
    }

    diagnostic_registrar(DIAG_ARGUMENTO);

    fprintf(
        stderr,
        "Error: contiene() solo funciona con lista o diccionario, no %s.\n",
        value_type_name(container.type)
    );

    return 0;
}

/*
 * ==========================
 * TEXTO
 * ==========================
 */

/*
 * unir(lista, separador)
 *
 * Todos los elementos deben ser
 * texto: unir() no convierte por su
 * cuenta, para eso esta texto().
 */

static int builtin_unir(
    Value list,
    Value separator,
    Value *result
) {

    if (list.type != VALUE_LIST) {

        diagnostic_registrar(DIAG_ARGUMENTO);

        fprintf(
            stderr,
            "Error: unir() necesita una lista, no %s.\n",
            value_type_name(list.type)
        );

        return 0;
    }

    if (separator.type != VALUE_STRING) {

        diagnostic_registrar(DIAG_ARGUMENTO);

        fprintf(
            stderr,
            "Error: el separador de unir() debe ser un texto, no %s.\n",
            value_type_name(separator.type)
        );

        return 0;
    }

    int count = value_list_count(list);

    /*
     * Primero comprobamos que TODOS
     * sean texto, para no construir
     * nada a medias.
     */

    for (int i = 0; i < count; i++) {

        Value *item = value_list_at(list, i);

        if (item->type != VALUE_STRING) {

            diagnostic_registrar(DIAG_ARGUMENTO);

            fprintf(
                stderr,
                "Error: unir() necesita una lista de textos, "
                "pero el elemento %d es %s.\n",
                i,
                value_type_name(item->type)
            );

            return 0;
        }
    }

    size_t separator_length =
        strlen(separator.data.string);

    size_t total = 1;

    for (int i = 0; i < count; i++) {

        total +=
            strlen(
                value_list_at(list, i)->data.string
            );

        if (i > 0) {
            total += separator_length;
        }
    }

    char *joined = malloc(total);

    if (joined == NULL) {

        fprintf(
            stderr,
            "Error: no se pudo unir el texto.\n"
        );

        return 0;
    }

    joined[0] = '\0';

    for (int i = 0; i < count; i++) {

        if (i > 0) {
            strcat(joined, separator.data.string);
        }

        strcat(
            joined,
            value_list_at(list, i)->data.string
        );
    }

    *result = value_string(joined);

    free(joined);

    return 1;
}

/*
 * separar(texto, separador)
 *
 * "a,b,c" con ","  →  ["a", "b", "c"]
 *
 * Si el separador no aparece, sale
 * una lista con el texto entero.
 * El separador vacio es un error:
 * no habria forma de trocear.
 */

static int builtin_separar(
    Value text,
    Value separator,
    Value *result
) {

    if (text.type != VALUE_STRING) {

        diagnostic_registrar(DIAG_ARGUMENTO);

        fprintf(
            stderr,
            "Error: separar() necesita un texto, no %s.\n",
            value_type_name(text.type)
        );

        return 0;
    }

    if (separator.type != VALUE_STRING) {

        diagnostic_registrar(DIAG_ARGUMENTO);

        fprintf(
            stderr,
            "Error: el separador de separar() debe ser un texto, no %s.\n",
            value_type_name(separator.type)
        );

        return 0;
    }

    const char *source = text.data.string;
    const char *mark = separator.data.string;

    size_t mark_length = strlen(mark);

    if (mark_length == 0) {

        fprintf(
            stderr,
            "Error: el separador de separar() no puede estar vacío.\n"
        );

        return 0;
    }

    Value pieces = value_list();

    if (pieces.data.list == NULL) {

        fprintf(
            stderr,
            "Error: no se pudo crear la lista.\n"
        );

        return 0;
    }

    const char *start = source;

    for (;;) {

        const char *found = strstr(start, mark);

        size_t length =
            found == NULL
                ? strlen(start)
                : (size_t)(found - start);

        char *piece = malloc(length + 1);

        if (piece == NULL) {

            fprintf(
                stderr,
                "Error: no se pudo separar el texto.\n"
            );

            value_free(&pieces);

            return 0;
        }

        memcpy(piece, start, length);

        piece[length] = '\0';

        Value item = value_string(piece);

        free(piece);

        if (!value_list_push(&pieces, item)) {

            fprintf(
                stderr,
                "Error: no se pudo ampliar la lista.\n"
            );

            value_free(&item);
            value_free(&pieces);

            return 0;
        }

        if (found == NULL) {
            break;
        }

        start = found + mark_length;
    }

    *result = pieces;

    return 1;
}

/*
 * mayusculas(texto) / minusculas(texto)
 *
 * Solo ASCII: las letras acentuadas
 * son varios bytes en UTF-8 y se
 * dejan tal cual.
 */

static int builtin_cambiar_caja(
    const char *name,
    Value text,
    int a_mayusculas,
    Value *result
) {

    if (text.type != VALUE_STRING) {

        diagnostic_registrar(DIAG_ARGUMENTO);

        fprintf(
            stderr,
            "Error: %s() solo funciona con texto, no %s.\n",
            name,
            value_type_name(text.type)
        );

        return 0;
    }

    Value copy = value_copy(text);

    if (copy.data.string == NULL) {
        *result = copy;
        return 1;
    }

    for (char *c = copy.data.string; *c != '\0'; c++) {

        *c =
            a_mayusculas
                ? (char)toupper((unsigned char)*c)
                : (char)tolower((unsigned char)*c);
    }

    *result = copy;

    return 1;
}

/*
 * ==========================
 * NUMEROS
 * ==========================
 */

/*
 * Un double solo se puede convertir
 * a numero si cabe: pasarse es
 * comportamiento indefinido en C.
 */

static int decimal_cabe_en_numero(double value) {

    return
        value >= (double)INT_MIN &&
        value <= (double)INT_MAX;
}

/*
 * absoluto(valor)
 *
 * CONSERVA el tipo:
 *
 * absoluto(-5)    → 5     (numero)
 * absoluto(-2.5)  → 2.5   (decimal)
 */

static int builtin_absoluto(
    Value value,
    Value *result
) {

    if (value.type == VALUE_NUMBER) {

        int n = value.data.number;

        if (n == INT_MIN) {

            fprintf(
                stderr,
                "Error: absoluto() no puede representar el opuesto de %d.\n",
                n
            );

            return 0;
        }

        *result = value_number(n < 0 ? -n : n);

        return 1;
    }

    if (value.type == VALUE_DECIMAL) {

        double d = value.data.decimal;

        *result = value_decimal(d < 0.0 ? -d : d);

        return 1;
    }

    diagnostic_registrar(DIAG_ARGUMENTO);

    fprintf(
        stderr,
        "Error: absoluto() solo funciona con numero o decimal, no %s.\n",
        value_type_name(value.type)
    );

    return 0;
}

/*
 * redondear(valor)
 *
 * Siempre devuelve NUMERO: redondear
 * sirve justo para obtener un entero.
 *
 * redondear(5)    → 5
 * redondear(2.4)  → 2
 * redondear(2.5)  → 3
 * redondear(-2.5) → -3
 *
 * Medio hacia afuera del cero, sin
 * math.h.
 */

static int builtin_redondear(
    Value value,
    Value *result
) {

    if (value.type == VALUE_NUMBER) {

        *result = value_number(value.data.number);

        return 1;
    }

    if (value.type == VALUE_DECIMAL) {

        double d = value.data.decimal;

        /*
         * El cast a long long solo es
         * seguro si el valor cabe
         * holgadamente: convertir un
         * double enorme a entero es
         * comportamiento indefinido.
         *
         * Se comprueba ANTES de
         * convertir, no despues.
         */

        if (!(d >= -9.0e18 && d <= 9.0e18)) {

            diagnostic_registrar(DIAG_ARGUMENTO);

            fprintf(
                stderr,
                "Error: redondear() no puede convertir %g a numero.\n",
                d
            );

            return 0;
        }

        long long rounded =
            d >= 0.0
                ? (long long)(d + 0.5)
                : (long long)(d - 0.5);

        if (
            rounded < (long long)INT_MIN ||
            rounded > (long long)INT_MAX
        ) {

            diagnostic_registrar(DIAG_ARGUMENTO);

            fprintf(
                stderr,
                "Error: redondear() no puede convertir %g a numero.\n",
                d
            );

            return 0;
        }

        *result = value_number((int)rounded);

        return 1;
    }

    diagnostic_registrar(DIAG_ARGUMENTO);

    fprintf(
        stderr,
        "Error: redondear() solo funciona con numero o decimal, no %s.\n",
        value_type_name(value.type)
    );

    return 0;
}

/*
 * ==========================
 * DICCIONARIOS
 * ==========================
 *
 * claves() y valores() devuelven
 * listas nuevas en ORDEN DE
 * INSERCION, el mismo que recorre
 * 'para cada'. Asi claves(d)[i] y
 * valores(d)[i] se corresponden.
 */

static int builtin_claves_valores(
    const char *name,
    Value dictionary,
    int solo_claves,
    Value *result
) {

    if (dictionary.type != VALUE_DICTIONARY) {

        diagnostic_registrar(DIAG_ARGUMENTO);

        fprintf(
            stderr,
            "Error: %s() solo funciona con diccionarios, no %s.\n",
            name,
            value_type_name(dictionary.type)
        );

        return 0;
    }

    Value out = value_list();

    if (out.data.list == NULL) {

        fprintf(
            stderr,
            "Error: no se pudo crear la lista.\n"
        );

        return 0;
    }

    int count =
        value_dictionary_count(dictionary);

    for (int i = 0; i < count; i++) {

        const char *key =
            value_dictionary_key_at(dictionary, i);

        Value item =
            solo_claves
                ? value_string(key)
                : value_copy(
                      *value_dictionary_at(
                          dictionary,
                          key
                      )
                  );

        if (!value_list_push(&out, item)) {

            fprintf(
                stderr,
                "Error: no se pudo ampliar la lista.\n"
            );

            value_free(&item);
            value_free(&out);

            return 0;
        }
    }

    *result = out;

    return 1;
}

/*
 * ==========================
 * LLAMAR A UNA INCORPORADA
 * ==========================
 */

static int call_builtin(
    const Builtin *builtin,
    ASTNode *node,
    Environment *environment,
    FunctionTable *functions,
    Value *result
) {

    int received =
        node->data.function_call.argument_count;

    if (received != builtin->arity) {

        fprintf(
            stderr,
            "Error: la función '%s' esperaba %d argumento%s, "
            "pero recibió %d.\n",
            builtin->name,
            builtin->arity,
            builtin->arity == 1 ? "" : "s",
            received
        );

        return 0;
    }

    /*
     * ==========================
     * MUTADORAS
     * ==========================
     *
     * agregar(lista, valor)
     * eliminar(lista, indice)
     * eliminar(diccionario, clave)
     */

    if (builtin->mutates_first) {

        Value *target = NULL;

        if (
            !resolve_lvalue(
                node->data.function_call.arguments[0],
                environment,
                functions,
                &target
            )
        ) {

            return 0;
        }

        Value argument;

        if (
            !evaluate_expression(
                node->data.function_call.arguments[1],
                environment,
                functions,
                &argument
            )
        ) {

            return 0;
        }

        if (strcmp(builtin->name, "agregar") == 0) {

            if (target->type != VALUE_LIST) {

                diagnostic_registrar(DIAG_ARGUMENTO);

                fprintf(
                    stderr,
                    "Error: agregar() solo funciona con listas, no %s.\n",
                    value_type_name(target->type)
                );

                value_free(&argument);

                return 0;
            }

            /*
             * La lista se queda con el
             * valor: no hay que
             * liberarlo aqui.
             */

            if (!value_list_push(target, argument)) {

                fprintf(
                    stderr,
                    "Error: no se pudo ampliar la lista.\n"
                );

                value_free(&argument);

                return 0;
            }

            *result = value_null();

            return 1;
        }

        /*
         * eliminar()
         */

        if (target->type == VALUE_LIST) {

            if (argument.type != VALUE_NUMBER) {

                diagnostic_registrar(DIAG_TIPO);

                fprintf(
                    stderr,
                    "Error: el índice debe ser un numero, no %s.\n",
                    value_type_name(argument.type)
                );

                value_free(&argument);

                return 0;
            }

            int position = argument.data.number;

            Value removed;

            if (
                !value_list_remove(
                    target,
                    position,
                    &removed
                )
            ) {

                diagnostic_registrar(DIAG_INDICE);

                fprintf(
                    stderr,
                    "Error: índice %d fuera de rango. "
                    "La lista tiene %d elemento%s.\n",
                    position,
                    value_list_count(*target),
                    value_list_count(*target) == 1
                        ? ""
                        : "s"
                );

                value_free(&argument);

                return 0;
            }

            value_free(&argument);

            /*
             * El elemento eliminado pasa
             * a ser del resultado.
             */

            *result = removed;

            return 1;
        }

        if (target->type == VALUE_DICTIONARY) {

            if (argument.type != VALUE_STRING) {

                diagnostic_registrar(DIAG_TIPO);

                fprintf(
                    stderr,
                    "Error: la clave debe ser un texto, no %s.\n",
                    value_type_name(argument.type)
                );

                value_free(&argument);

                return 0;
            }

            Value removed;

            if (
                !value_dictionary_remove(
                    target,
                    argument.data.string,
                    &removed
                )
            ) {

                diagnostic_registrar(DIAG_CLAVE);

                fprintf(
                    stderr,
                    "Error: la clave '%s' no existe en el diccionario.\n",
                    argument.data.string
                );

                value_free(&argument);

                return 0;
            }

            value_free(&argument);

            *result = removed;

            return 1;
        }

        diagnostic_registrar(DIAG_ARGUMENTO);

        fprintf(
            stderr,
            "Error: eliminar() solo funciona con lista o diccionario, no %s.\n",
            value_type_name(target->type)
        );

        value_free(&argument);

        return 0;
    }

    /*
     * ==========================
     * PURAS
     * ==========================
     */

    Value arguments[2];
    int evaluated = 0;

    for (int i = 0; i < builtin->arity; i++) {

        if (
            !evaluate_expression(
                node->data.function_call.arguments[i],
                environment,
                functions,
                &arguments[i]
            )
        ) {

            for (int j = 0; j < evaluated; j++) {
                value_free(&arguments[j]);
            }

            return 0;
        }

        evaluated++;
    }

    int success = 0;

    if (strcmp(builtin->name, "largo") == 0) {

        success =
            builtin_largo(arguments[0], result);
    }
    else if (strcmp(builtin->name, "tipo") == 0) {

        *result =
            value_string(
                value_type_name(arguments[0].type)
            );

        success = 1;
    }
    else if (strcmp(builtin->name, "texto") == 0) {

        success =
            builtin_texto(arguments[0], result);
    }
    else if (strcmp(builtin->name, "numero") == 0) {

        success =
            builtin_numero(arguments[0], result);
    }
    else if (strcmp(builtin->name, "decimal") == 0) {

        success =
            builtin_decimal(arguments[0], result);
    }
    else if (strcmp(builtin->name, "contiene") == 0) {

        success =
            builtin_contiene(
                arguments[0],
                arguments[1],
                result
            );
    }
    else if (strcmp(builtin->name, "unir") == 0) {

        success =
            builtin_unir(
                arguments[0],
                arguments[1],
                result
            );
    }
    else if (strcmp(builtin->name, "separar") == 0) {

        success =
            builtin_separar(
                arguments[0],
                arguments[1],
                result
            );
    }
    else if (strcmp(builtin->name, "mayusculas") == 0) {

        success =
            builtin_cambiar_caja(
                "mayusculas",
                arguments[0],
                1,
                result
            );
    }
    else if (strcmp(builtin->name, "minusculas") == 0) {

        success =
            builtin_cambiar_caja(
                "minusculas",
                arguments[0],
                0,
                result
            );
    }
    else if (strcmp(builtin->name, "absoluto") == 0) {

        success =
            builtin_absoluto(arguments[0], result);
    }
    else if (strcmp(builtin->name, "redondear") == 0) {

        success =
            builtin_redondear(arguments[0], result);
    }
    else if (strcmp(builtin->name, "claves") == 0) {

        success =
            builtin_claves_valores(
                "claves",
                arguments[0],
                1,
                result
            );
    }
    else if (strcmp(builtin->name, "valores") == 0) {

        success =
            builtin_claves_valores(
                "valores",
                arguments[0],
                0,
                result
            );
    }

    for (int i = 0; i < evaluated; i++) {
        value_free(&arguments[i]);
    }

    return success;
}

static int call_function(
    ASTNode *node,
    Environment *environment,
    FunctionTable *functions,
    Value *result
) {

    const char *name =
        node->data.function_call.name;

    /*
     * Las incorporadas van primero:
     * declarar una funcion con uno
     * de estos nombres se rechaza en
     * la declaracion, asi que aqui no
     * puede haber ambiguedad.
     */

    const Builtin *builtin =
        builtin_find(name);

    if (builtin != NULL) {

        return call_builtin(
            builtin,
            node,
            environment,
            functions,
            result
        );
    }

    Function *function =
        function_table_find(
            functions,
            name
        );

    if (function == NULL) {

        diagnostic_registrar(DIAG_FUNCION_NO_DEFINIDA);

        fprintf(
            stderr,
            "Error: la función '%s' no existe.\n",
            name
        );

        return 0;
    }

    /*
     * ==========================
     * COMPROBAR LA ARIDAD
     * ==========================
     */

    int expected =
        function->parameter_count;

    int received =
        node->data.function_call.argument_count;

    if (expected != received) {

        fprintf(
            stderr,
            "Error: la función '%s' esperaba %d argumento%s, "
            "pero recibió %d.\n",
            name,
            expected,
            expected == 1 ? "" : "s",
            received
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
     * ==========================
     * ENLAZAR ARGUMENTOS
     * ==========================
     *
     * Cada argumento se evalúa en
     * el entorno del LLAMADOR, y el
     * resultado se declara como
     * variable LOCAL de la llamada.
     *
     * Cada argumento se evalúa una
     * sola vez.
     */

    for (int i = 0; i < expected; i++) {

        Value argument;

        if (
            !evaluate_expression(
                node->data.function_call.arguments[i],
                environment,
                functions,
                &argument
            )
        ) {

            environment_free(local);

            return 0;
        }

        /*
         * environment_set se queda
         * con el Value.
         */

        environment_set(
            local,
            function->parameters[i],
            argument
        );
    }

    /*
     * El cuerpo es un bloque
     * como cualquier otro.
     */

    ExecutionResult body =
        execute_block(
            function->body,
            local,
            functions
        );

    /*
     * ==========================
     * DESTRUIR EL SCOPE LOCAL
     * ==========================
     *
     * El Value retornado NO
     * pertenece a este entorno:
     * evaluate_expression siempre
     * produce valores propios
     * (environment_get devuelve una
     * COPIA).
     *
     * Por eso se puede destruir el
     * entorno antes de usar el
     * valor, incluso si es un texto.
     */

    environment_free(local);

    if (!body.success) {

        value_free(&body.value);

        return 0;
    }

    if (body.returned) {

        *result = body.value;

        return 1;
    }

    /*
     * Una función que termina sin
     * 'retornar' produce nulo.
     *
     * Un 'romper' o 'continuar'
     * sueltos no pueden llegar aquí
     * porque el parser los rechaza
     * fuera de un bucle; y si
     * llegaran, su value es nulo y
     * no se pierde nada.
     */

    value_free(&body.value);

    *result = value_null();

    return 1;
}

/*
 * ==========================
 * RESOLVER UN HUECO INDEXADO
 * ==========================
 *
 * numeros[1] = 99
 * matriz[1][0] = 5
 *
 * Devuelve un puntero al Value que
 * hay DENTRO de la lista, para
 * poder sustituirlo sin copiar la
 * lista entera.
 *
 * Se apoya en environment_get_ref
 * para la variable de base y luego
 * baja por los índices. Como una
 * lista guarda sus elementos en un
 * array propio, cada nivel da un
 * puntero válido al siguiente, y
 * matriz[1][0] sale solo.
 */

static int resolve_index_slot(
    ASTNode *node,
    Environment *environment,
    FunctionTable *functions,
    int create,
    Value **slot
) {

    ASTNode *object_node =
        node->data.index.object;

    Value *container = NULL;

    /*
     * Base: una variable.
     */

    if (object_node->type == AST_IDENTIFIER) {

        container =
            environment_get_ref(
                environment,
                object_node->data.identifier
            );

        if (container == NULL) {

            diagnostic_registrar(DIAG_VARIABLE_NO_DEFINIDA);

            fprintf(
                stderr,
                "Error: variable '%s' no existe.\n",
                object_node->data.identifier
            );

            return 0;
        }
    }
    else if (object_node->type == AST_INDEX) {

        /*
         * Nivel intermedio: bajamos
         * un escalón más.
         *
         * create = 0: los niveles
         * intermedios NUNCA se crean.
         * persona["a"]["b"] = 1 exige
         * que "a" ya exista, para no
         * dejar claves a medias si
         * algo falla después.
         */

        if (
            !resolve_index_slot(
                object_node,
                environment,
                functions,
                0,
                &container
            )
        ) {

            return 0;
        }
    }
    else {

        /*
         * crear()[0] = 5 no tiene
         * sentido: el valor es
         * temporal y se perdería.
         */

        fprintf(
            stderr,
            "Error: solo se puede asignar por índice sobre una variable.\n"
        );

        return 0;
    }

    if (
        container->type != VALUE_LIST &&
        container->type != VALUE_DICTIONARY
    ) {

        diagnostic_registrar(DIAG_TIPO);

        fprintf(
            stderr,
            "Error: solo se puede indexar una lista o un diccionario, no %s.\n",
            value_type_name(container->type)
        );

        return 0;
    }

    Value index;

    if (
        !evaluate_expression(
            node->data.index.index,
            environment,
            functions,
            &index
        )
    ) {

        return 0;
    }

    /*
     * ==========================
     * LISTA
     * ==========================
     *
     * El indice debe existir: no se
     * puede crecer una lista
     * asignando fuera de rango.
     */

    if (container->type == VALUE_LIST) {

        if (index.type != VALUE_NUMBER) {

            diagnostic_registrar(DIAG_TIPO);

            fprintf(
                stderr,
                "Error: el índice debe ser un numero, no %s.\n",
                value_type_name(index.type)
            );

            value_free(&index);

            return 0;
        }

        int position = index.data.number;

        Value *item =
            value_list_at(*container, position);

        value_free(&index);

        if (item == NULL) {

            diagnostic_registrar(DIAG_INDICE);

            fprintf(
                stderr,
                "Error: índice %d fuera de rango. "
                "La lista tiene %d elemento%s.\n",
                position,
                value_list_count(*container),
                value_list_count(*container) == 1
                    ? ""
                    : "s"
            );

            return 0;
        }

        *slot = item;

        return 1;
    }

    /*
     * ==========================
     * DICCIONARIO
     * ==========================
     *
     * Aqui SI se puede crear la
     * clave, pero solo en el ultimo
     * nivel (create == 1):
     *
     * persona["pais"] = "Colombia"
     */

    if (index.type != VALUE_STRING) {

        diagnostic_registrar(DIAG_TIPO);

        fprintf(
            stderr,
            "Error: la clave debe ser un texto, no %s.\n",
            value_type_name(index.type)
        );

        value_free(&index);

        return 0;
    }

    Value *item =
        value_dictionary_at(
            *container,
            index.data.string
        );

    if (item == NULL) {

        if (!create) {

            diagnostic_registrar(DIAG_CLAVE);

            fprintf(
                stderr,
                "Error: la clave '%s' no existe en el diccionario.\n",
                index.data.string
            );

            value_free(&index);

            return 0;
        }

        /*
         * Clave nueva: se crea vacia
         * y se vuelve a buscar, porque
         * insertar puede haber movido
         * el array de pares.
         */

        if (
            !value_dictionary_set(
                container,
                index.data.string,
                value_null()
            )
        ) {

            fprintf(
                stderr,
                "Error: no se pudo crear la clave '%s'.\n",
                index.data.string
            );

            value_free(&index);

            return 0;
        }

        item =
            value_dictionary_at(
                *container,
                index.data.string
            );
    }

    value_free(&index);

    *slot = item;

    return 1;
}

static ExecutionResult execute_statement(
    ASTNode *node,
    Environment *environment,
    FunctionTable *functions
) {

    if (node == NULL) {
        return execution_error();
    }

    /*
     * ==========================
     * ASIGNACIÓN POR ÍNDICE
     * ==========================
     *
     * numeros[1] = 99
     */

    if (node->type == AST_INDEX_ASSIGNMENT) {

        /*
         * El nuevo valor se evalúa
         * ANTES de tocar la lista:
         * si falla, la lista queda
         * intacta.
         */

        Value value;

        if (
            !evaluate_expression(
                node->data.index_assignment.value,
                environment,
                functions,
                &value
            )
        ) {

            return execution_error();
        }

        Value *slot = NULL;

        if (
            !resolve_index_slot(
                node->data.index_assignment.target,
                environment,
                functions,
                1,
                &slot
            )
        ) {

            value_free(&value);

            return execution_error();
        }

        /*
         * Fuera el valor anterior,
         * dentro el nuevo.
         */

        value_free(slot);

        *slot = value;

        return execution_ok();
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
                functions,
                &value
            )
        ) {

            return execution_error();
        }

        environment_set(
            environment,
            node->data.variable.name,
            value
        );

        return execution_ok();
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

            diagnostic_registrar(DIAG_VARIABLE_NO_DEFINIDA);

            fprintf(
                stderr,
                "Error: la variable '%s' no existe.\n",
                node->data.assignment.name
            );

            return execution_error();
        }

        /*
         * 2. Evaluar la expresión.
         */

        Value value;

        if (
            !evaluate_expression(
                node->data.assignment.value,
                environment,
                functions,
                &value
            )
        ) {

            return execution_error();
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

            return execution_error();
        }

        return execution_ok();
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
                functions,
                &value
            )
        ) {

            return execution_error();
        }

        value_print(value);

        printf("\n");

        value_free(&value);

        return execution_ok();
    }

    /*
     * ==========================
     * RETORNAR
     * ==========================
     *
     * retornar
     * retornar a + b
     *
     * Marca 'returned' y deja que
     * execute_block corte todos los
     * bloques hasta la llamada.
     */

    if (node->type == AST_RETURN) {

        /*
         * 'retornar' a secas
         * devuelve nulo.
         */

        if (node->data.return_value == NULL) {

            return execution_return(
                value_null()
            );
        }

        Value value;

        if (
            !evaluate_expression(
                node->data.return_value,
                environment,
                functions,
                &value
            )
        ) {

            return execution_error();
        }

        /*
         * El Value pasa a ser de
         * quien reciba este
         * ExecutionResult.
         */

        return execution_return(value);
    }

    /*
     * ==========================
     * ROMPER / CONTINUAR
     * ==========================
     *
     * Solo levantan la señal. El
     * 'mientras' que las recoja
     * decide qué hacer.
     */

    if (node->type == AST_BREAK) {

        return execution_break();
    }

    if (node->type == AST_CONTINUE) {

        return execution_continue();
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
                functions,
                &condition
            )
        ) {

            return execution_error();
        }

        int is_true =
            value_is_truthy(condition);

        value_free(&condition);

        /*
         * Si el bloque retorna, el
         * ExecutionResult sube tal
         * cual: el 'retornar' sale
         * también del 'si'.
         */

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

            return execution_ok();
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
                    functions,
                    &condition
                )
            ) {

                return execution_error();
            }

            int is_true =
                value_is_truthy(condition);

            value_free(&condition);

            /*
             * 2. Si es falso, salimos.
             */

            if (!is_true) {
                return execution_ok();
            }

            /*
             * 3. Ejecutar el cuerpo.
             */

            ExecutionResult body =
                execute_block(
                    node->data.while_statement.body,
                    environment,
                    functions
                );

            /*
             * Error o 'retornar':
             * NO se consumen aquí.
             * Suben para salir también
             * de la función.
             */

            if (!body.success || body.returned) {
                return body;
            }

            /*
             * 'romper': lo consume
             * ESTE bucle y termina.
             *
             * Al devolver execution_ok
             * la señal no sale de aquí,
             * que es lo que hace que en
             * bucles anidados solo
             * afecte al más interno.
             */

            if (body.broke) {

                value_free(&body.value);

                return execution_ok();
            }

            /*
             * 'continuar': lo consume
             * ESTE bucle y sigue con la
             * vuelta siguiente.
             *
             * No hace falta nada más:
             * basta con no propagarlo.
             */

            value_free(&body.value);
        }
    }

    /*
     * ==========================
     * PARA CADA
     * ==========================
     *
     * para cada numero en numeros
     *     ...
     * fin
     */

    if (node->type == AST_FOR_EACH) {

        /*
         * La lista se evalúa UNA vez.
         *
         * Como las listas son de
         * copia profunda, esto es una
         * instantánea nuestra: nadie
         * puede cambiarla desde el
         * cuerpo a mitad de recorrido.
         */

        Value iterable;

        if (
            !evaluate_expression(
                node->data.for_each.iterable,
                environment,
                functions,
                &iterable
            )
        ) {

            return execution_error();
        }

        if (
            iterable.type != VALUE_LIST &&
            iterable.type != VALUE_DICTIONARY
        ) {

            fprintf(
                stderr,
                "Error: 'para cada' solo puede recorrer una lista o un diccionario, no %s.\n",
                value_type_name(iterable.type)
            );

            value_free(&iterable);

            return execution_error();
        }

        /*
         * En una lista se recorren los
         * ELEMENTOS; en un diccionario,
         * las CLAVES, en orden de
         * insercion.
         */

        int es_diccionario =
            iterable.type == VALUE_DICTIONARY;

        int count =
            es_diccionario
                ? value_dictionary_count(iterable)
                : value_list_count(iterable);

        for (int i = 0; i < count; i++) {

            /*
             * La variable del bucle es
             * una COPIA del elemento,
             * así que modificarla no
             * altera la lista.
             */

            Value elemento;

            if (es_diccionario) {

                elemento =
                    value_string(
                        value_dictionary_key_at(
                            iterable,
                            i
                        )
                    );
            }
            else {

                elemento =
                    value_copy(
                        *value_list_at(iterable, i)
                    );
            }

            environment_set(
                environment,
                node->data.for_each.variable,
                elemento
            );

            ExecutionResult body =
                execute_block(
                    node->data.for_each.body,
                    environment,
                    functions
                );

            /*
             * Error o 'retornar' suben.
             *
             * El Value retornado es
             * independiente de la lista,
             * así que liberarla aquí es
             * seguro.
             */

            if (!body.success || body.returned) {

                value_free(&iterable);

                return body;
            }

            /*
             * 'romper' lo consume este
             * bucle.
             */

            if (body.broke) {

                value_free(&body.value);

                break;
            }

            /*
             * 'continuar' también: basta
             * con no propagarlo y pasar
             * al elemento siguiente.
             */

            value_free(&body.value);
        }

        value_free(&iterable);

        return execution_ok();
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
         * Las incorporadas no se
         * pueden tapar.
         */

        if (builtin_find(name) != NULL) {

            fprintf(
                stderr,
                "Error: '%s' es una función incorporada de TzLang.\n",
                name
            );

            return execution_error();
        }

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

            return execution_error();
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
                node->data.function_declaration.parameters,
                node->data.function_declaration.parameter_count,
                node->data.function_declaration.body,
                environment_global(environment)
            )
        ) {

            fprintf(
                stderr,
                "Error: no se pudo declarar la función '%s'.\n",
                name
            );

            return execution_error();
        }

        return execution_ok();
    }

    /*
     * ==========================
     * LLAMADA A FUNCIÓN
     * ==========================
     *
     * saludar()
     */

    if (node->type == AST_FUNCTION_CALL) {

        /*
         * Como instrucción, el valor
         * que devuelva la función se
         * descarta, pero hay que
         * liberarlo igualmente por si
         * es un texto.
         */

        Value discarded;

        if (
            !call_function(
                node,
                environment,
                functions,
                &discarded
            )
        ) {

            return execution_error();
        }

        value_free(&discarded);

        return execution_ok();
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

    return execution_error();
}

/*
 * ==========================
 * EJECUTAR BLOQUE
 * ==========================
 *
 * Lo usan el programa completo,
 * los cuerpos de 'si' y 'sino',
 * el de 'mientras' y el de las
 * funciones.
 *
 * Se detiene en cuanto una
 * instrucción falla O retorna.
 */

static ExecutionResult execute_block(
    ASTNode *block,
    Environment *environment,
    FunctionTable *functions
) {

    if (
        block == NULL ||
        block->type != AST_PROGRAM
    ) {

        return execution_error();
    }

    for (
        int i = 0;
        i < block->data.program.count;
        i++
    ) {

        ExecutionResult result =
            execute_statement(
                block->data.program.statements[i],
                environment,
                functions
            );

        /*
         * Error, retorno, romper o
         * continuar cortan el bloque
         * y suben tal cual.
         *
         * El bloque NO decide nada:
         * solo transporta. Quien
         * consume cada señal es el
         * 'mientras' o la llamada.
         */

        if (execution_interrupts(result)) {
            return result;
        }

        value_free(&result.value);
    }

    return execution_ok();
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

    ExecutionResult result =
        execute_block(
            program,
            environment,
            functions
        );

    /*
     * El parser ya rechaza un
     * 'retornar' fuera de una
     * función, así que aquí
     * returned no debería llegar a 1.
     *
     * Liberamos el Value igualmente:
     * es inofensivo y no depende de
     * esa suposición.
     */

    value_free(&result.value);

    if (!result.success) {

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