#ifndef TZLANG_INTERPRETER_H
#define TZLANG_INTERPRETER_H

#include "../ast/ast.h"

typedef struct {
    char *name;
    int value;
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
void environment_free(Environment *environment);

/*
 * Guardar o actualizar una variable
 */
void environment_set(
    Environment *environment,
    const char *name,
    int value
);

/*
 * Buscar una variable
 */
int environment_get(
    Environment *environment,
    const char *name,
    int *value
);

/*
 * Ejecutar un programa AST
 */
int interpreter_run(
    ASTNode *program
);

#endif