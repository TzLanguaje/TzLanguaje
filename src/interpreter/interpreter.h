#ifndef TZLANG_INTERPRETER_H
#define TZLANG_INTERPRETER_H

#include "../ast/ast.h"
#include "../runtime/value.h"

typedef struct {
    char *name;
    Value value;
} Variable;

typedef struct {
    Variable *variables;
    int count;
    int capacity;
} Environment;

/*
 * Crear entorno
 */
Environment *environment_create(void);

/*
 * Liberar entorno
 */
void environment_free(
    Environment *environment
);

/*
 * Guardar o actualizar variable
 */
void environment_set(
    Environment *environment,
    const char *name,
    Value value
);

/*
 * Buscar variable
 */
int environment_get(
    Environment *environment,
    const char *name,
    Value *value
);

/*
 * ¿La variable existe?
 */
int environment_has(
    Environment *environment,
    const char *name
);

/*
 * Asignar a una variable EXISTENTE.
 *
 * A diferencia de environment_set,
 * nunca crea variables nuevas:
 * devuelve 0 si no existe.
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
 * Ejecutar programa
 */
int interpreter_run(
    ASTNode *program
);

#endif