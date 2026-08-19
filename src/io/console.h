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

#endif
