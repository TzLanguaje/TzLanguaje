#ifndef TZLANG_INTERPRETER_H
#define TZLANG_INTERPRETER_H

#include "../ast/ast.h"
#include "../runtime/value.h"

typedef struct {
    char *name;
    Value value;
} Variable;

/*
 * ==========================
 * ENVIRONMENT
 * ==========================
 *
 * Los scopes forman una cadena:
 *
 * local → parent → ... → global
 *
 * El global tiene parent NULL.
 *
 * Cada llamada a función crea un
 * scope local y lo destruye al
 * terminar.
 */

typedef struct Environment {
    Variable *variables;
    int count;
    int capacity;
    struct Environment *parent;
} Environment;

/*
 * Crear entorno.
 *
 * parent == NULL  →  entorno global.
 *
 * El entorno NUNCA es dueño de su
 * padre: liberarlo no toca la cadena.
 */
Environment *environment_create(
    Environment *parent
);

/*
 * Liberar entorno.
 *
 * Libera sus variables, nombres y
 * Values. No toca el padre.
 */
void environment_free(
    Environment *environment
);

/*
 * DECLARAR una variable.
 *
 * variable x = 10
 *
 * Actúa SOLO sobre este scope, así
 * que declarar dentro de una función
 * tapa (shadow) a la global sin
 * modificarla.
 */
void environment_set(
    Environment *environment,
    const char *name,
    Value value
);

/*
 * Leer una variable.
 *
 * Busca local → parent → ... → global.
 * Devuelve una copia del Value.
 */
int environment_get(
    Environment *environment,
    const char *name,
    Value *value
);

/*
 * ¿La variable existe en la cadena?
 */
int environment_has(
    Environment *environment,
    const char *name
);

/*
 * ASIGNAR a una variable EXISTENTE.
 *
 * edad = 25
 *
 * Busca local → parent → ... → global
 * y modifica la variable donde la
 * encuentre. Nunca crea variables:
 * devuelve 0 si no existe en ningún
 * scope.
 *
 * Libera el Value anterior y se
 * queda con el nuevo.
 */
int environment_assign(
    Environment *environment,
    const char *name,
    Value value
);

/*
 * ==========================
 * FUNCIONES
 * ==========================
 *
 * IMPORTANTE:
 *
 * 'body' apunta al AST, NO es
 * una copia. El dueño sigue
 * siendo el AST, así que la
 * tabla nunca lo libera.
 *
 * 'closure' es el entorno donde
 * la función fue DEFINIDA. Es lo
 * que da scope LÉXICO: al llamar,
 * el scope local cuelga de aquí,
 * no de quien llama.
 *
 * Tampoco es propiedad de la
 * tabla: la tabla nunca lo libera.
 *
 * Hoy siempre es el entorno global
 * (ver function_table_declare).
 */

typedef struct {
    char *name;
    ASTNode *body;
    Environment *closure;
} Function;

typedef struct {
    Function *functions;
    int count;
    int capacity;
} FunctionTable;

/*
 * Crear tabla de funciones
 */
FunctionTable *function_table_create(void);

/*
 * Liberar tabla de funciones
 *
 * Libera los nombres, nunca
 * los cuerpos.
 */
void function_table_free(
    FunctionTable *table
);

/*
 * Registrar una función.
 *
 * 'closure' es el entorno de
 * definición y debe sobrevivir a
 * todas las llamadas.
 *
 * No comprueba duplicados:
 * eso lo decide quien llama
 * con function_table_find.
 */
int function_table_declare(
    FunctionTable *table,
    const char *name,
    ASTNode *body,
    Environment *closure
);

/*
 * Buscar una función.
 *
 * Devuelve NULL si no existe.
 *
 * Devuelve la Function entera
 * porque llamar necesita tanto
 * el cuerpo como el closure.
 */
Function *function_table_find(
    FunctionTable *table,
    const char *name
);

/*
 * Ejecutar programa
 */
int interpreter_run(
    ASTNode *program
);

#endif