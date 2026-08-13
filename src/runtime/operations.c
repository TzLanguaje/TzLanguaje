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