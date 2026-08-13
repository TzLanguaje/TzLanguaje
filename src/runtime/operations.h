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

#endif