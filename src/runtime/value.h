#ifndef TZLANG_VALUE_H
#define TZLANG_VALUE_H

typedef enum {
    VALUE_NUMBER,
    VALUE_DECIMAL,
    VALUE_STRING,
    VALUE_BOOLEAN,
    VALUE_NULL
} ValueType;

typedef struct {
    ValueType type;

    union {
        int number;
        double decimal;
        char *string;
        int boolean;
    } data;

} Value;

/*
 * Crear valores
 */

Value value_number(int value);

Value value_decimal(double value);

Value value_string(const char *value);

Value value_boolean(int value);

Value value_null(void);

/*
 * Liberar memoria de un Value
 */

void value_free(Value *value);

/*
 * Mostrar un Value
 */

void value_print(Value value);

/*
 * Obtener nombre del tipo
 */

const char *value_type_name(ValueType type);

#endif