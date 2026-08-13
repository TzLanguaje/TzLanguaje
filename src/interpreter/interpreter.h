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
 * Ejecutar programa
 */
int interpreter_run(
    ASTNode *program
);

#endif