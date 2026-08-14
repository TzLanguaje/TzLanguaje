#include "file.h"

#include <stdio.h>
#include <stdlib.h>

char *read_file(const char *path) {

    if (path == NULL) {
        return NULL;
    }

    /*
     * Modo binario: no queremos que
     * la plataforma traduzca nada.
     * El lexer ya trata '\r' como
     * espacio.
     */

    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        return NULL;
    }

    /*
     * ==========================
     * TAMAÑO
     * ==========================
     */

    if (fseek(file, 0, SEEK_END) != 0) {

        fclose(file);

        return NULL;
    }

    long size = ftell(file);

    /*
     * ftell devuelve -1 en lo que no
     * se puede medir, por ejemplo un
     * directorio o una tuberia.
     */

    if (size < 0) {

        fclose(file);

        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {

        fclose(file);

        return NULL;
    }

    /*
     * ==========================
     * LEER
     * ==========================
     *
     * +1 para el terminador.
     */

    char *buffer = malloc((size_t)size + 1);

    if (buffer == NULL) {

        fclose(file);

        return NULL;
    }

    size_t leidos =
        fread(buffer, 1, (size_t)size, file);

    /*
     * fread puede leer menos de lo
     * anunciado. Si fue por un error
     * de verdad, abortamos; si no,
     * cerramos el texto donde toque.
     */

    if (ferror(file)) {

        free(buffer);

        fclose(file);

        return NULL;
    }

    buffer[leidos] = '\0';

    fclose(file);

    return buffer;
}
