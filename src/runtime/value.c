#include "value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

Value value_number(int value) {

    Value value_result;

    value_result.type = VALUE_NUMBER;
    value_result.data.number = value;

    return value_result;
}

Value value_decimal(double value) {

    Value value_result;

    value_result.type = VALUE_DECIMAL;
    value_result.data.decimal = value;

    return value_result;
}

Value value_string(const char *value) {

    Value value_result;

    value_result.type = VALUE_STRING;

    value_result.data.string =
        copy_string(value);

    return value_result;
}

Value value_boolean(int value) {

    Value value_result;

    value_result.type = VALUE_BOOLEAN;

    value_result.data.boolean =
        value ? 1 : 0;

    return value_result;
}

Value value_null(void) {

    Value value_result;

    value_result.type = VALUE_NULL;

    return value_result;
}

void value_free(Value *value) {

    if (value == NULL) {
        return;
    }

    if (value->type == VALUE_STRING) {
        free(value->data.string);
        value->data.string = NULL;
    }

}

const char *value_type_name(
    ValueType type
) {

    switch (type) {

        case VALUE_NUMBER:
            return "numero";

        case VALUE_DECIMAL:
            return "decimal";

        case VALUE_STRING:
            return "texto";

        case VALUE_BOOLEAN:
            return "booleano";

        case VALUE_NULL:
            return "nulo";

        default:
            return "desconocido";
    }
}

int value_is_truthy(Value value) {

    switch (value.type) {

        case VALUE_BOOLEAN:
            return value.data.boolean ? 1 : 0;

        case VALUE_NUMBER:
            return value.data.number != 0;

        case VALUE_DECIMAL:
            return value.data.decimal != 0.0;

        case VALUE_STRING:

            return
                value.data.string != NULL &&
                value.data.string[0] != '\0';

        case VALUE_NULL:
            return 0;

        default:
            return 0;
    }
}

void value_print(Value value) {

    switch (value.type) {

        case VALUE_NUMBER:

            printf(
                "%d",
                value.data.number
            );

            break;

        case VALUE_DECIMAL:

            printf(
                "%g",
                value.data.decimal
            );

            break;

        case VALUE_STRING:

            printf(
                "%s",
                value.data.string
            );

            break;

        case VALUE_BOOLEAN:

            printf(
                "%s",
                value.data.boolean
                    ? "verdadero"
                    : "falso"
            );

            break;

        case VALUE_NULL:

            printf("nulo");

            break;

        default:

            printf("<?>");

            break;
    }
}