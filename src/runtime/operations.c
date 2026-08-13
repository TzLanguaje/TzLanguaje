#include "operations.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ==========================
 * SUMA
 * ==========================
 */

int operation_add(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    /*
     * NUMBER + NUMBER
     */

    if (
        left.type == VALUE_NUMBER &&
        right.type == VALUE_NUMBER
    ) {

        *result =
            value_number(
                left.data.number +
                right.data.number
            );

        return 1;
    }

    /*
     * DECIMAL + DECIMAL
     */

    if (
        left.type == VALUE_DECIMAL &&
        right.type == VALUE_DECIMAL
    ) {

        *result =
            value_decimal(
                left.data.decimal +
                right.data.decimal
            );

        return 1;
    }

    /*
     * NUMBER + DECIMAL
     */

    if (
        left.type == VALUE_NUMBER &&
        right.type == VALUE_DECIMAL
    ) {

        *result =
            value_decimal(
                left.data.number +
                right.data.decimal
            );

        return 1;
    }

    /*
     * DECIMAL + NUMBER
     */

    if (
        left.type == VALUE_DECIMAL &&
        right.type == VALUE_NUMBER
    ) {

        *result =
            value_decimal(
                left.data.decimal +
                right.data.number
            );

        return 1;
    }

    /*
     * STRING + STRING
     *
     * "Hola " + "TzLang"
     */

    if (
        left.type == VALUE_STRING &&
        right.type == VALUE_STRING
    ) {

        size_t left_length =
            strlen(left.data.string);

        size_t right_length =
            strlen(right.data.string);

        char *combined =
            malloc(
                left_length +
                right_length +
                1
            );

        if (combined == NULL) {
            return 0;
        }

        strcpy(
            combined,
            left.data.string
        );

        strcat(
            combined,
            right.data.string
        );

        *result =
            value_string(
                combined
            );

        free(combined);

        return 1;
    }

    fprintf(
        stderr,
        "Error: no se puede sumar %s con %s.\n",
        value_type_name(left.type),
        value_type_name(right.type)
    );

    return 0;
}

/*
 * ==========================
 * RESTA
 * ==========================
 */

int operation_subtract(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    /*
     * NUMBER - NUMBER
     */

    if (
        left.type == VALUE_NUMBER &&
        right.type == VALUE_NUMBER
    ) {

        *result =
            value_number(
                left.data.number -
                right.data.number
            );

        return 1;
    }

    /*
     * DECIMAL - DECIMAL
     */

    if (
        left.type == VALUE_DECIMAL &&
        right.type == VALUE_DECIMAL
    ) {

        *result =
            value_decimal(
                left.data.decimal -
                right.data.decimal
            );

        return 1;
    }

    /*
     * NUMBER - DECIMAL
     */

    if (
        left.type == VALUE_NUMBER &&
        right.type == VALUE_DECIMAL
    ) {

        *result =
            value_decimal(
                left.data.number -
                right.data.decimal
            );

        return 1;
    }

    /*
     * DECIMAL - NUMBER
     */

    if (
        left.type == VALUE_DECIMAL &&
        right.type == VALUE_NUMBER
    ) {

        *result =
            value_decimal(
                left.data.decimal -
                right.data.number
            );

        return 1;
    }

    fprintf(
        stderr,
        "Error: no se puede restar %s con %s.\n",
        value_type_name(left.type),
        value_type_name(right.type)
    );

    return 0;
}

/*
 * ==========================
 * MULTIPLICACIÓN
 * ==========================
 */

int operation_multiply(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    /*
     * NUMBER * NUMBER
     */

    if (
        left.type == VALUE_NUMBER &&
        right.type == VALUE_NUMBER
    ) {

        *result =
            value_number(
                left.data.number *
                right.data.number
            );

        return 1;
    }

    /*
     * DECIMAL * DECIMAL
     */

    if (
        left.type == VALUE_DECIMAL &&
        right.type == VALUE_DECIMAL
    ) {

        *result =
            value_decimal(
                left.data.decimal *
                right.data.decimal
            );

        return 1;
    }

    /*
     * NUMBER * DECIMAL
     */

    if (
        left.type == VALUE_NUMBER &&
        right.type == VALUE_DECIMAL
    ) {

        *result =
            value_decimal(
                left.data.number *
                right.data.decimal
            );

        return 1;
    }

    /*
     * DECIMAL * NUMBER
     */

    if (
        left.type == VALUE_DECIMAL &&
        right.type == VALUE_NUMBER
    ) {

        *result =
            value_decimal(
                left.data.decimal *
                right.data.number
            );

        return 1;
    }

    fprintf(
        stderr,
        "Error: no se puede multiplicar %s con %s.\n",
        value_type_name(left.type),
        value_type_name(right.type)
    );

    return 0;
}

/*
 * ==========================
 * DIVISIÓN
 * ==========================
 */

int operation_divide(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    /*
     * NUMBER / NUMBER
     */

    if (
        left.type == VALUE_NUMBER &&
        right.type == VALUE_NUMBER
    ) {

        if (right.data.number == 0) {

            fprintf(
                stderr,
                "Error: división por cero.\n"
            );

            return 0;
        }

        *result =
            value_number(
                left.data.number /
                right.data.number
            );

        return 1;
    }

    /*
     * DECIMAL / DECIMAL
     */

    if (
        left.type == VALUE_DECIMAL &&
        right.type == VALUE_DECIMAL
    ) {

        if (right.data.decimal == 0.0) {

            fprintf(
                stderr,
                "Error: división por cero.\n"
            );

            return 0;
        }

        *result =
            value_decimal(
                left.data.decimal /
                right.data.decimal
            );

        return 1;
    }

    /*
     * NUMBER / DECIMAL
     */

    if (
        left.type == VALUE_NUMBER &&
        right.type == VALUE_DECIMAL
    ) {

        if (right.data.decimal == 0.0) {

            fprintf(
                stderr,
                "Error: división por cero.\n"
            );

            return 0;
        }

        *result =
            value_decimal(
                left.data.number /
                right.data.decimal
            );

        return 1;
    }

    /*
     * DECIMAL / NUMBER
     */

    if (
        left.type == VALUE_DECIMAL &&
        right.type == VALUE_NUMBER
    ) {

        if (right.data.number == 0) {

            fprintf(
                stderr,
                "Error: división por cero.\n"
            );

            return 0;
        }

        *result =
            value_decimal(
                left.data.decimal /
                right.data.number
            );

        return 1;
    }

    fprintf(
        stderr,
        "Error: no se puede dividir %s entre %s.\n",
        value_type_name(left.type),
        value_type_name(right.type)
    );

    return 0;
}

/*
 * ==========================
 * UTILIDADES DE COMPARACIÓN
 * ==========================
 */

/*
 * ¿El valor es numérico?
 *
 * NUMBER
 * DECIMAL
 */

static int is_numeric(Value value) {

    return
        value.type == VALUE_NUMBER ||
        value.type == VALUE_DECIMAL;
}

/*
 * Convierte un valor numérico
 * a double.
 *
 * Así podemos comparar
 * NUMBER con DECIMAL.
 */

static double as_double(Value value) {

    if (value.type == VALUE_DECIMAL) {
        return value.data.decimal;
    }

    return (double)value.data.number;
}

/*
 * ==========================
 * ORDEN
 * ==========================
 *
 * Devuelve en comparison:
 *
 * -1  →  left < right
 *  0  →  left == right
 *  1  →  left > right
 *
 * Devuelve 0 si los tipos
 * no se pueden ordenar.
 */

static int compare_ordered(
    Value left,
    Value right,
    int *comparison
) {

    if (comparison == NULL) {
        return 0;
    }

    /*
     * NUMBER con NUMBER
     */

    if (
        left.type == VALUE_NUMBER &&
        right.type == VALUE_NUMBER
    ) {

        if (
            left.data.number <
            right.data.number
        ) {
            *comparison = -1;
        }
        else if (
            left.data.number >
            right.data.number
        ) {
            *comparison = 1;
        }
        else {
            *comparison = 0;
        }

        return 1;
    }

    /*
     * DECIMAL con DECIMAL
     * NUMBER  con DECIMAL
     * DECIMAL con NUMBER
     */

    if (
        is_numeric(left) &&
        is_numeric(right)
    ) {

        double left_number =
            as_double(left);

        double right_number =
            as_double(right);

        if (left_number < right_number) {
            *comparison = -1;
        }
        else if (left_number > right_number) {
            *comparison = 1;
        }
        else {
            *comparison = 0;
        }

        return 1;
    }

    /*
     * BOOLEAN con BOOLEAN
     *
     * falso < verdadero
     */

    if (
        left.type == VALUE_BOOLEAN &&
        right.type == VALUE_BOOLEAN
    ) {

        if (
            left.data.boolean <
            right.data.boolean
        ) {
            *comparison = -1;
        }
        else if (
            left.data.boolean >
            right.data.boolean
        ) {
            *comparison = 1;
        }
        else {
            *comparison = 0;
        }

        return 1;
    }

    /*
     * El resto NO se puede ordenar.
     *
     * Incluye texto:
     *
     * "hola" > "mundo"
     */

    fprintf(
        stderr,
        "Error: no se puede comparar %s con %s.\n",
        value_type_name(left.type),
        value_type_name(right.type)
    );

    return 0;
}

/*
 * ==========================
 * IGUALDAD
 * ==========================
 *
 * A diferencia del orden,
 * comparar tipos distintos
 * no es un error:
 *
 * simplemente no son iguales.
 */

static int values_equal(
    Value left,
    Value right
) {

    /*
     * NUMBER  con NUMBER
     * DECIMAL con DECIMAL
     * NUMBER  con DECIMAL
     * DECIMAL con NUMBER
     */

    if (
        is_numeric(left) &&
        is_numeric(right)
    ) {

        if (
            left.type == VALUE_NUMBER &&
            right.type == VALUE_NUMBER
        ) {

            return
                left.data.number ==
                right.data.number;
        }

        return
            as_double(left) ==
            as_double(right);
    }

    /*
     * BOOLEAN con BOOLEAN
     */

    if (
        left.type == VALUE_BOOLEAN &&
        right.type == VALUE_BOOLEAN
    ) {

        return
            left.data.boolean ==
            right.data.boolean;
    }

    /*
     * STRING con STRING
     *
     * "hola" == "hola"
     */

    if (
        left.type == VALUE_STRING &&
        right.type == VALUE_STRING
    ) {

        return
            strcmp(
                left.data.string,
                right.data.string
            ) == 0;
    }

    /*
     * NULL con NULL
     */

    if (
        left.type == VALUE_NULL &&
        right.type == VALUE_NULL
    ) {

        return 1;
    }

    /*
     * Tipos distintos
     */

    return 0;
}

/*
 * ==========================
 * MAYOR QUE
 * ==========================
 *
 * 10 > 5
 */

int operation_greater(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    int comparison = 0;

    if (
        !compare_ordered(
            left,
            right,
            &comparison
        )
    ) {

        return 0;
    }

    *result =
        value_boolean(comparison > 0);

    return 1;
}

/*
 * ==========================
 * MENOR QUE
 * ==========================
 *
 * 10 < 5
 */

int operation_less(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    int comparison = 0;

    if (
        !compare_ordered(
            left,
            right,
            &comparison
        )
    ) {

        return 0;
    }

    *result =
        value_boolean(comparison < 0);

    return 1;
}

/*
 * ==========================
 * IGUAL A
 * ==========================
 *
 * 10 == 10
 * "hola" == "hola"
 */

int operation_equal(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    *result =
        value_boolean(
            values_equal(left, right)
        );

    return 1;
}

/*
 * ==========================
 * DISTINTO DE
 * ==========================
 *
 * 10 != 5
 * "hola" != "mundo"
 */

int operation_not_equal(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    *result =
        value_boolean(
            !values_equal(left, right)
        );

    return 1;
}

/*
 * ==========================
 * MAYOR O IGUAL QUE
 * ==========================
 *
 * edad es mayor o igual que 18
 */

int operation_greater_equal(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    int comparison = 0;

    if (
        !compare_ordered(
            left,
            right,
            &comparison
        )
    ) {

        return 0;
    }

    *result =
        value_boolean(comparison >= 0);

    return 1;
}

/*
 * ==========================
 * MENOR O IGUAL QUE
 * ==========================
 *
 * edad es menor o igual que 18
 */

int operation_less_equal(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    int comparison = 0;

    if (
        !compare_ordered(
            left,
            right,
            &comparison
        )
    ) {

        return 0;
    }

    *result =
        value_boolean(comparison <= 0);

    return 1;
}

/*
 * ==========================
 * Y
 * ==========================
 *
 * edad es mayor que 18 y documento
 */

int operation_and(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    *result =
        value_boolean(
            value_is_truthy(left) &&
            value_is_truthy(right)
        );

    return 1;
}

/*
 * ==========================
 * O
 * ==========================
 *
 * edad es mayor que 18 o permiso
 */

int operation_or(
    Value left,
    Value right,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    *result =
        value_boolean(
            value_is_truthy(left) ||
            value_is_truthy(right)
        );

    return 1;
}

/*
 * ==========================
 * NO
 * ==========================
 *
 * no activo
 */

int operation_not(
    Value operand,
    Value *result
) {

    if (result == NULL) {
        return 0;
    }

    *result =
        value_boolean(
            !value_is_truthy(operand)
        );

    return 1;
}