#ifndef TZLANG_OPERATIONS_H
#define TZLANG_OPERATIONS_H

#include "value.h"

/*
 * ==========================
 * OPERACIONES ARITMÉTICAS
 * ==========================
 */

int operation_add(
    Value left,
    Value right,
    Value *result
);

int operation_subtract(
    Value left,
    Value right,
    Value *result
);

int operation_multiply(
    Value left,
    Value right,
    Value *result
);

int operation_divide(
    Value left,
    Value right,
    Value *result
);

/*
 * ==========================
 * OPERACIONES DE COMPARACIÓN
 * ==========================
 *
 * El resultado siempre es
 * un VALUE_BOOLEAN:
 *
 * verdadero
 * falso
 */

int operation_greater(
    Value left,
    Value right,
    Value *result
);

int operation_less(
    Value left,
    Value right,
    Value *result
);

int operation_equal(
    Value left,
    Value right,
    Value *result
);

int operation_not_equal(
    Value left,
    Value right,
    Value *result
);

int operation_greater_equal(
    Value left,
    Value right,
    Value *result
);

int operation_less_equal(
    Value left,
    Value right,
    Value *result
);

/*
 * ==========================
 * OPERACIONES LÓGICAS
 * ==========================
 *
 * y
 * o
 * no
 *
 * Trabajan sobre la "verdad"
 * del valor (value_is_truthy).
 */

int operation_and(
    Value left,
    Value right,
    Value *result
);

int operation_or(
    Value left,
    Value right,
    Value *result
);

int operation_not(
    Value operand,
    Value *result
);

/*
 * ==========================
 * MENOS UNARIO
 * ==========================
 *
 * -5    → -5   (numero)
 * -2.5  → -2.5 (decimal)
 *
 * Conserva el tipo. Cualquier otro
 * tipo es error: no hay conversion
 * implicita.
 */

int operation_negate(
    Value operand,
    Value *result
);

#endif