#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    /*
     * ==========================
     * BOM UTF-8
     * ==========================
     *
     * Muchos editores guardan los
     * archivos empezando por
     *
     *     EF BB BF
     *
     * Esos tres bytes no son parte
     * del programa, asi que se
     * descartan aqui y el lexer no
     * llega a verlos.
     *
     * Se desplaza el contenido en
     * vez de devolver buffer + 3,
     * porque quien llama tiene que
     * poder hacer free() del mismo
     * puntero que recibio.
     *
     * El resto del contenido queda
     * byte a byte igual.
     */

    if (
        leidos >= 3 &&
        (unsigned char)buffer[0] == 0xEF &&
        (unsigned char)buffer[1] == 0xBB &&
        (unsigned char)buffer[2] == 0xBF
    ) {

        /*
         * +1 para arrastrar tambien
         * el terminador.
         */

        memmove(
            buffer,
            buffer + 3,
            leidos - 3 + 1
        );
    }

    return buffer;
}
