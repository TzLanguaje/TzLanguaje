#include "value.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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
        copy_string(
            value == NULL ? "" : value
        );

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

/*
 * ==========================
 * LISTAS
 * ==========================
 */

Value value_list(void) {

    Value value_result;

    value_result.type = VALUE_LIST;

    List *list = malloc(sizeof(List));

    if (list == NULL) {

        value_result.data.list = NULL;

        return value_result;
    }

    list->count = 0;
    list->capacity = 4;

    list->items =
        malloc(
            sizeof(Value) * list->capacity
        );

    if (list->items == NULL) {

        free(list);

        value_result.data.list = NULL;

        return value_result;
    }

    value_result.data.list = list;

    return value_result;
}

int value_list_push(
    Value *list,
    Value item
) {

    if (
        list == NULL ||
        list->type != VALUE_LIST ||
        list->data.list == NULL
    ) {
        return 0;
    }

    List *target = list->data.list;

    if (target->count >= target->capacity) {

        int new_capacity =
            target->capacity * 2;

        Value *grown =
            realloc(
                target->items,
                sizeof(Value) * new_capacity
            );

        if (grown == NULL) {
            return 0;
        }

        target->items = grown;
        target->capacity = new_capacity;
    }

    /*
     * La lista se queda con el item
     * tal cual: no se copia.
     */

    target->items[target->count] = item;

    target->count++;

    return 1;
}

int value_list_count(Value list) {

    if (
        list.type != VALUE_LIST ||
        list.data.list == NULL
    ) {
        return 0;
    }

    return list.data.list->count;
}

Value *value_list_at(
    Value list,
    int index
) {

    if (
        list.type != VALUE_LIST ||
        list.data.list == NULL
    ) {
        return NULL;
    }

    if (
        index < 0 ||
        index >= list.data.list->count
    ) {
        return NULL;
    }

    return &list.data.list->items[index];
}

int value_list_remove(
    Value *list,
    int index,
    Value *removed
) {

    if (
        list == NULL ||
        removed == NULL ||
        list->type != VALUE_LIST ||
        list->data.list == NULL
    ) {
        return 0;
    }

    List *target = list->data.list;

    if (index < 0 || index >= target->count) {
        return 0;
    }

    /*
     * El elemento sale tal cual: no
     * se copia ni se libera aqui.
     */

    *removed = target->items[index];

    for (
        int i = index;
        i < target->count - 1;
        i++
    ) {
        target->items[i] = target->items[i + 1];
    }

    target->count--;

    return 1;
}

/*
 * ==========================
 * DICCIONARIOS
 * ==========================
 */

Value value_dictionary(void) {

    Value value_result;

    value_result.type = VALUE_DICTIONARY;

    Dictionary *dictionary =
        malloc(sizeof(Dictionary));

    if (dictionary == NULL) {

        value_result.data.dictionary = NULL;

        return value_result;
    }

    dictionary->count = 0;
    dictionary->capacity = 4;

    dictionary->entries =
        malloc(
            sizeof(DictionaryEntry) *
            dictionary->capacity
        );

    if (dictionary->entries == NULL) {

        free(dictionary);

        value_result.data.dictionary = NULL;

        return value_result;
    }

    value_result.data.dictionary = dictionary;

    return value_result;
}

/*
 * Búsqueda lineal por CONTENIDO de
 * la clave.
 */

static int dictionary_find(
    Value dictionary,
    const char *key
) {

    if (
        dictionary.type != VALUE_DICTIONARY ||
        dictionary.data.dictionary == NULL ||
        key == NULL
    ) {
        return -1;
    }

    Dictionary *target =
        dictionary.data.dictionary;

    for (int i = 0; i < target->count; i++) {

        if (
            strcmp(
                target->entries[i].key,
                key
            ) == 0
        ) {

            return i;
        }
    }

    return -1;
}

int value_dictionary_set(
    Value *dictionary,
    const char *key,
    Value item
) {

    if (
        dictionary == NULL ||
        dictionary->type != VALUE_DICTIONARY ||
        dictionary->data.dictionary == NULL ||
        key == NULL
    ) {
        return 0;
    }

    Dictionary *target =
        dictionary->data.dictionary;

    /*
     * La clave ya existe: se
     * reutiliza y se libera el
     * valor anterior.
     */

    int index =
        dictionary_find(*dictionary, key);

    if (index >= 0) {

        value_free(&target->entries[index].value);

        target->entries[index].value = item;

        return 1;
    }

    /*
     * Clave nueva
     */

    if (target->count >= target->capacity) {

        int new_capacity =
            target->capacity * 2;

        DictionaryEntry *grown =
            realloc(
                target->entries,
                sizeof(DictionaryEntry) *
                new_capacity
            );

        if (grown == NULL) {
            return 0;
        }

        target->entries = grown;
        target->capacity = new_capacity;
    }

    char *stored_key = copy_string(key);

    if (stored_key == NULL) {
        return 0;
    }

    target->entries[target->count].key =
        stored_key;

    target->entries[target->count].value =
        item;

    target->count++;

    return 1;
}

Value *value_dictionary_at(
    Value dictionary,
    const char *key
) {

    int index =
        dictionary_find(dictionary, key);

    if (index < 0) {
        return NULL;
    }

    return
        &dictionary.data.dictionary
            ->entries[index].value;
}

int value_dictionary_contains(
    Value dictionary,
    const char *key
) {

    return
        dictionary_find(dictionary, key) >= 0;
}

int value_dictionary_count(Value dictionary) {

    if (
        dictionary.type != VALUE_DICTIONARY ||
        dictionary.data.dictionary == NULL
    ) {
        return 0;
    }

    return dictionary.data.dictionary->count;
}

int value_dictionary_remove(
    Value *dictionary,
    const char *key,
    Value *removed
) {

    if (
        dictionary == NULL ||
        removed == NULL ||
        dictionary->type != VALUE_DICTIONARY ||
        dictionary->data.dictionary == NULL
    ) {
        return 0;
    }

    int index =
        dictionary_find(*dictionary, key);

    if (index < 0) {
        return 0;
    }

    Dictionary *target =
        dictionary->data.dictionary;

    /*
     * La clave es nuestra: se libera.
     * El valor se entrega a quien
     * llama.
     */

    free(target->entries[index].key);

    *removed = target->entries[index].value;

    for (
        int i = index;
        i < target->count - 1;
        i++
    ) {
        target->entries[i] =
            target->entries[i + 1];
    }

    target->count--;

    return 1;
}

const char *value_dictionary_key_at(
    Value dictionary,
    int index
) {

    if (
        dictionary.type != VALUE_DICTIONARY ||
        dictionary.data.dictionary == NULL
    ) {
        return NULL;
    }

    if (
        index < 0 ||
        index >= dictionary.data.dictionary->count
    ) {
        return NULL;
    }

    return
        dictionary.data.dictionary
            ->entries[index].key;
}

/*
 * ==========================
 * COPIA PROFUNDA
 * ==========================
 */

Value value_copy(Value source) {

    switch (source.type) {

        case VALUE_NUMBER:

            return value_number(
                source.data.number
            );

        case VALUE_DECIMAL:

            return value_decimal(
                source.data.decimal
            );

        case VALUE_STRING:

            return value_string(
                source.data.string
            );

        case VALUE_BOOLEAN:

            return value_boolean(
                source.data.boolean
            );

        case VALUE_LIST: {

            Value copy = value_list();

            if (
                copy.data.list == NULL ||
                source.data.list == NULL
            ) {
                return copy;
            }

            for (
                int i = 0;
                i < source.data.list->count;
                i++
            ) {

                Value item =
                    value_copy(
                        source.data.list->items[i]
                    );

                if (
                    !value_list_push(
                        &copy,
                        item
                    )
                ) {

                    /*
                     * Si no cabe, el item
                     * sigue siendo nuestro.
                     */

                    value_free(&item);

                    return copy;
                }
            }

            return copy;
        }

        case VALUE_DICTIONARY: {

            Value copy = value_dictionary();

            if (
                copy.data.dictionary == NULL ||
                source.data.dictionary == NULL
            ) {
                return copy;
            }

            for (
                int i = 0;
                i < source.data.dictionary->count;
                i++
            ) {

                Value item =
                    value_copy(
                        source.data.dictionary
                            ->entries[i].value
                    );

                if (
                    !value_dictionary_set(
                        &copy,
                        source.data.dictionary
                            ->entries[i].key,
                        item
                    )
                ) {

                    value_free(&item);

                    return copy;
                }
            }

            return copy;
        }

        case VALUE_NULL:
        default:

            return value_null();
    }
}

/*
 * ==========================
 * LIBERAR
 * ==========================
 */

void value_free(Value *value) {

    if (value == NULL) {
        return;
    }

    if (value->type == VALUE_STRING) {

        free(value->data.string);

        value->data.string = NULL;

        return;
    }

    if (value->type == VALUE_LIST) {

        List *list = value->data.list;

        if (list == NULL) {
            return;
        }

        /*
         * 1. Cada elemento, de forma
         *    recursiva: así una lista
         *    de listas se libera
         *    entera.
         */

        for (
            int i = 0;
            i < list->count;
            i++
        ) {
            value_free(&list->items[i]);
        }

        /*
         * 2. El array de elementos.
         */

        free(list->items);

        /*
         * 3. La estructura List.
         */

        free(list);

        value->data.list = NULL;

        return;
    }
    if (value->type == VALUE_DICTIONARY) {

        Dictionary *dictionary =
            value->data.dictionary;

        if (dictionary == NULL) {
            return;
        }

        for (
            int i = 0;
            i < dictionary->count;
            i++
        ) {

            /*
             * 1. La clave
             */

            free(dictionary->entries[i].key);

            /*
             * 2. El valor, recursivo
             */

            value_free(
                &dictionary->entries[i].value
            );
        }

        /*
         * 3. El array de pares
         */

        free(dictionary->entries);

        /*
         * 4. La estructura Dictionary
         */

        free(dictionary);

        value->data.dictionary = NULL;

        return;
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

        case VALUE_LIST:
            return "lista";

        case VALUE_DICTIONARY:
            return "diccionario";

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

        case VALUE_LIST:

            /*
             * Lista vacía → falso
             * Lista con datos → verdadero
             */

            return value_list_count(value) > 0;

        case VALUE_DICTIONARY:

            /*
             * {} → falso
             * con datos → verdadero
             */

            return value_dictionary_count(value) > 0;

        case VALUE_NULL:
            return 0;

        default:
            return 0;
    }
}

/*
 * ==========================
 * IMPRIMIR
 * ==========================
 *
 * Dentro de una lista los textos
 * van entre comillas para que
 * [1, "2"] no se confunda con
 * [1, 2].
 *
 * Suelto, un texto se imprime tal
 * cual:
 *
 * imprimir "Hola"  →  Hola
 */

/*
 * ==========================
 * REPRESENTACION EN TEXTO
 * ==========================
 *
 * Una sola funcion construye como se
 * ve un valor, y tanto imprimir como
 * texto() usan esa.
 *
 * Antes se imprimia directamente con
 * printf, asi que texto() no tenia de
 * donde sacar la representacion de una
 * lista y se rendia con un error. Al
 * construirla en memoria las dos
 * comparten la misma, y no pueden
 * separarse con el tiempo.
 */

typedef struct {
    char  *datos;
    size_t usado;
    size_t capacidad;
} Buffer;

static int buffer_crecer(Buffer *b, size_t hace_falta) {

    size_t nueva;
    char *mas;

    if (b->usado + hace_falta + 1 <= b->capacidad) {
        return 1;
    }

    nueva = b->capacidad == 0 ? 64 : b->capacidad;

    while (nueva < b->usado + hace_falta + 1) {
        nueva *= 2;
    }

    mas = realloc(b->datos, nueva);

    if (mas == NULL) {
        return 0;
    }

    b->datos = mas;
    b->capacidad = nueva;

    return 1;
}

static int buffer_texto(Buffer *b, const char *texto) {

    size_t largo;

    if (texto == NULL) {
        texto = "";
    }

    largo = strlen(texto);

    if (!buffer_crecer(b, largo)) {
        return 0;
    }

    memcpy(b->datos + b->usado, texto, largo);

    b->usado += largo;
    b->datos[b->usado] = '\0';

    return 1;
}

/*
 * Los numeros caben de sobra en 64
 * caracteres, asi que se formatean
 * aparte y se anaden como texto.
 */

static int buffer_numero(Buffer *b, const char *formato, ...) {

    char temporal[64];
    va_list args;

    va_start(args, formato);
    vsnprintf(temporal, sizeof(temporal), formato, args);
    va_end(args);

    return buffer_texto(b, temporal);
}

static int value_render(
    Value value,
    int inside_list,
    Buffer *salida
) {

    switch (value.type) {

        case VALUE_NUMBER:
            return buffer_numero(salida, "%d", value.data.number);

        case VALUE_DECIMAL:

            /*
             * %g a secas corta a 6
             * cifras significativas:
             * 3.14159265358979 salia
             * "3.14159" y 3141592.65
             * salia "3.14159e+06".
             *
             * 15 cifras conservan lo que
             * escribio la persona y
             * evitan la notacion
             * cientifica en cualquier
             * rango normal.
             *
             * No se usan 17 (las que
             * hacen falta para
             * reconstruir un double
             * exacto) porque entonces
             * asoma el ruido de la coma
             * flotante: 0.1 + 0.2 se
             * veria 0.30000000000000004,
             * que en un lenguaje para
             * aprender no ayuda a nadie.
             *
             * %g sigue quitando los
             * ceros finales, asi que 8.0
             * se sigue viendo "8".
             */

            return buffer_numero(salida, "%.15g", value.data.decimal);

        case VALUE_STRING:

            /*
             * Dentro de una lista o un
             * diccionario los textos van
             * entrecomillados, para que
             * se distinga [1, "1"].
             * Sueltos, no.
             */

            if (inside_list) {
                return buffer_texto(salida, "\"")
                    && buffer_texto(salida, value.data.string)
                    && buffer_texto(salida, "\"");
            }

            return buffer_texto(salida, value.data.string);

        case VALUE_BOOLEAN:
            return buffer_texto(
                salida,
                value.data.boolean ? "verdadero" : "falso"
            );

        case VALUE_NULL:
            return buffer_texto(salida, "nulo");

        case VALUE_LIST: {

            int count = value_list_count(value);
            int i;

            if (!buffer_texto(salida, "[")) {
                return 0;
            }

            for (i = 0; i < count; i++) {

                if (i > 0 && !buffer_texto(salida, ", ")) {
                    return 0;
                }

                if (!value_render(value.data.list->items[i], 1, salida)) {
                    return 0;
                }
            }

            return buffer_texto(salida, "]");
        }

        case VALUE_DICTIONARY: {

            int count = value_dictionary_count(value);
            int i;

            if (!buffer_texto(salida, "{")) {
                return 0;
            }

            for (i = 0; i < count; i++) {

                if (i > 0 && !buffer_texto(salida, ", ")) {
                    return 0;
                }

                if (!buffer_texto(salida, "\"")
                    || !buffer_texto(salida, value.data.dictionary->entries[i].key)
                    || !buffer_texto(salida, "\": ")) {
                    return 0;
                }

                if (!value_render(
                        value.data.dictionary->entries[i].value, 1, salida)) {
                    return 0;
                }
            }

            return buffer_texto(salida, "}");
        }

        default:
            return buffer_texto(salida, "<?>");
    }
}

char *value_to_text(Value value) {

    Buffer b;

    b.datos = NULL;
    b.usado = 0;
    b.capacidad = 0;

    if (!buffer_texto(&b, "")) {
        return NULL;
    }

    if (!value_render(value, 0, &b)) {
        free(b.datos);
        return NULL;
    }

    return b.datos;
}

void value_print(Value value) {

    char *texto = value_to_text(value);

    if (texto == NULL) {
        return;
    }

    fputs(texto, stdout);

    free(texto);
}

