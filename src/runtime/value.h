#ifndef TZLANG_VALUE_H
#define TZLANG_VALUE_H

typedef enum {
    VALUE_NUMBER,
    VALUE_DECIMAL,
    VALUE_STRING,
    VALUE_BOOLEAN,
    VALUE_NULL,
    VALUE_LIST,
    VALUE_DICTIONARY
} ValueType;

typedef struct Value Value;
typedef struct List List;
typedef struct Dictionary Dictionary;
typedef struct DictionaryEntry DictionaryEntry;

/*
 * ==========================
 * VALUE
 * ==========================
 *
 * SEMÁNTICA: COPIA PROFUNDA
 *
 * Todo Value es dueño único de su
 * memoria, igual que ya ocurría
 * con VALUE_STRING y VALUE_LIST.
 *
 * Nadie comparte una List ni un
 * Dictionary: copiar un Value
 * copia la estructura entera y su
 * contenido, recursivamente.
 *
 * Por eso no hace falta contar
 * referencias y no puede haber
 * aliasing ni double free.
 */

struct Value {
    ValueType type;

    union {
        int number;
        double decimal;
        char *string;
        int boolean;
        List *list;
        Dictionary *dictionary;
    } data;
};

/*
 * ==========================
 * LIST
 * ==========================
 *
 * Array dinámico de Value.
 *
 * La lista es dueña de sus
 * elementos.
 */

struct List {
    Value *items;
    int count;
    int capacity;
};

/*
 * ==========================
 * DICTIONARY
 * ==========================
 *
 * Array dinámico de pares
 * clave/valor, en ORDEN DE
 * INSERCIÓN.
 *
 * La búsqueda es lineal por
 * contenido de la clave (strcmp),
 * no por dirección. Para el tamaño
 * de diccionario de un lenguaje
 * educativo es de sobra, y a
 * cambio el orden de recorrido es
 * predecible.
 *
 * El diccionario es dueño de sus
 * claves Y de sus valores.
 */

struct DictionaryEntry {
    char *key;
    Value value;
};

struct Dictionary {
    DictionaryEntry *entries;
    int count;
    int capacity;
};

/*
 * Crear valores
 */

Value value_number(int value);

Value value_decimal(double value);

Value value_string(const char *value);

Value value_boolean(int value);

Value value_null(void);

/*
 * Crear una lista VACÍA.
 *
 * Si falta memoria devuelve un
 * VALUE_LIST con data.list NULL;
 * el resto de funciones lo
 * toleran.
 */

Value value_list(void);

/*
 * Añadir un elemento al final.
 *
 * La lista SE QUEDA con el item:
 * no lo copia. Quien llama no debe
 * liberarlo después.
 *
 * Devuelve 0 si falta memoria; en
 * ese caso el item sigue siendo de
 * quien llama.
 */

int value_list_push(
    Value *list,
    Value item
);

/*
 * Número de elementos.
 *
 * 0 si no es una lista.
 */

int value_list_count(Value list);

/*
 * Puntero al elemento 'index'.
 *
 * Devuelve NULL si no es lista o
 * si el índice está fuera de
 * rango.
 *
 * Apunta DENTRO de la lista, así
 * que sirve para modificarla en
 * el sitio (asignación por
 * índice).
 */

Value *value_list_at(
    Value list,
    int index
);

/*
 * ==========================
 * DICCIONARIOS
 * ==========================
 */

/*
 * Crear un diccionario VACÍO.
 *
 * Si falta memoria devuelve un
 * VALUE_DICTIONARY con
 * data.dictionary NULL; el resto
 * de funciones lo toleran.
 */

Value value_dictionary(void);

/*
 * Insertar o actualizar.
 *
 * PROPIEDAD:
 *
 * - La CLAVE se copia. El char*
 *   que se pasa sigue siendo de
 *   quien llama.
 *
 * - El VALOR no se copia: el
 *   diccionario SE QUEDA con él.
 *   Quien llama no debe liberarlo.
 *
 * - Si la clave YA existía, el
 *   valor anterior se libera aquí
 *   y la clave se reutiliza.
 *
 * Devuelve 0 si falta memoria; en
 * ese caso el item sigue siendo de
 * quien llama y el diccionario
 * queda intacto.
 */

int value_dictionary_set(
    Value *dictionary,
    const char *key,
    Value item
);

/*
 * Puntero al valor de 'key'.
 *
 * Devuelve NULL si no es un
 * diccionario o si la clave no
 * existe.
 *
 * Apunta DENTRO del diccionario,
 * así que sirve para modificarlo
 * en el sitio.
 *
 * Deja de ser válido si se inserta
 * una clave nueva (puede haber
 * realloc).
 */

Value *value_dictionary_at(
    Value dictionary,
    const char *key
);

/*
 * ¿Existe la clave?
 */

int value_dictionary_contains(
    Value dictionary,
    const char *key
);

/*
 * Número de pares.
 *
 * 0 si no es un diccionario.
 */

int value_dictionary_count(Value dictionary);

/*
 * Clave número 'index', en orden
 * de inserción.
 *
 * Lo usa 'para cada'.
 *
 * Devuelve NULL si está fuera de
 * rango. El char* pertenece al
 * diccionario: no se libera.
 */

const char *value_dictionary_key_at(
    Value dictionary,
    int index
);

/*
 * Quitar el elemento 'index'.
 *
 * PROPIEDAD: el elemento sale de
 * la lista y pasa a 'removed', que
 * es de quien llama.
 *
 * Los siguientes se desplazan, asi
 * que el orden se conserva.
 *
 * Devuelve 0 si no es lista o si
 * el indice esta fuera de rango.
 */

int value_list_remove(
    Value *list,
    int index,
    Value *removed
);

/*
 * Quitar la clave.
 *
 * PROPIEDAD: la clave se libera
 * aqui; el VALOR pasa a 'removed',
 * que es de quien llama.
 *
 * Los pares siguientes se
 * desplazan, asi que el orden de
 * insercion se conserva.
 *
 * Devuelve 0 si no es diccionario
 * o si la clave no existe.
 */

int value_dictionary_remove(
    Value *dictionary,
    const char *key,
    Value *removed
);

/*
 * Copia profunda.
 *
 * El resultado no comparte nada
 * con el original.
 */

Value value_copy(Value source);

/*
 * Liberar memoria de un Value
 *
 * En las listas libera, en este
 * orden:
 *
 * 1. cada Value contenido
 *    (recursivo)
 * 2. el array de elementos
 * 3. la estructura List
 *
 * En los diccionarios:
 *
 * 1. cada clave
 * 2. cada Value contenido
 *    (recursivo)
 * 3. el array de pares
 * 4. la estructura Dictionary
 */

void value_free(Value *value);

/*
 * Mostrar un Value
 */

/*
 * Devuelve como se ve un valor, en
 * memoria reservada que hay que
 * liberar con free().
 *
 * Es la MISMA representacion que
 * imprime value_print: las dos usan
 * esta funcion.
 *
 * Devuelve NULL si no hay memoria.
 */
char *value_to_text(Value value);

void value_print(Value value);

/*
 * ¿El valor cuenta como verdadero?
 *
 * Lo usan 'si', 'y', 'o' y 'no'.
 *
 * verdadero  → 1
 * falso      → 0
 * 0 / 0.0    → 0
 * ""         → 0
 * nulo       → 0
 * []         → 0
 * {}         → 0
 * el resto   → 1
 */

int value_is_truthy(Value value);

/*
 * Obtener nombre del tipo
 */

const char *value_type_name(ValueType type);

#endif
