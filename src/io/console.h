#ifndef TZLANG_IO_CONSOLE_H
#define TZLANG_IO_CONSOLE_H

/*
 * ==========================
 * CONSOLA
 * ==========================
 *
 * Prepara el terminal para que la
 * salida de TzLang se vea bien.
 *
 * Se llama UNA vez, al arrancar,
 * antes de imprimir nada.
 *
 * En macOS y Linux no hace nada: el
 * terminal ya habla UTF-8.
 */

void console_init(void);

/*
 * ¿Es stderr un terminal de verdad, o
 * esta redirigido a un archivo o a
 * una tuberia?
 *
 * Vive aqui, junto a console_init(),
 * porque la respuesta se pregunta de
 * forma distinta en cada sistema y
 * este es el modulo que aisla esas
 * diferencias.
 *
 * Devuelve 1 si es un terminal.
 */

int console_stderr_es_terminal(void);

#endif
