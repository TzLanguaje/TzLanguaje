#ifndef TZLANG_FILE_H
#define TZLANG_FILE_H

/*
 * ==========================
 * LECTURA DE ARCHIVOS
 * ==========================
 *
 * Lee un archivo COMPLETO y lo
 * devuelve como texto terminado
 * en '\0', listo para el lexer.
 *
 * Sin buffers de tamaño fijo: se
 * reserva segun el tamaño real del
 * archivo, asi que no hay limite
 * artificial de longitud.
 *
 * PROPIEDAD:
 *
 * El buffer devuelto es de quien
 * llama, que debe liberarlo con
 * free().
 *
 * Devuelve NULL si el archivo no
 * se puede abrir, medir o leer. En
 * ese caso no queda nada reservado.
 *
 * Un archivo VACIO no es un error:
 * devuelve una cadena vacia, que es
 * un programa valido sin
 * instrucciones.
 */

char *read_file(const char *path);

#endif
