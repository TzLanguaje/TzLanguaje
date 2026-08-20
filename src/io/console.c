#include "console.h"

/*
 * ==========================
 * POR QUE ESTO VIVE APARTE
 * ==========================
 *
 * <windows.h> es la unica cabecera
 * de plataforma de todo el proyecto,
 * y esta ENCERRADA en este archivo a
 * proposito.
 *
 * No es manía: winnt.h, que entra con
 * windows.h, declara un enumerador
 * llamado 'TokenType'. Nuestro lexer
 * define un tipo con ese mismo
 * nombre. Incluir windows.h en un
 * .c que ademas use lexer.h rompe la
 * compilacion entera con MSVC.
 *
 * Aqui no se incluye ninguna cabecera
 * del proyecto salvo la propia, asi
 * que no hay colision posible.
 */

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

void console_init(void) {

    /*
     * Los mensajes de TzLang llevan
     * acentos y enes, y salen en
     * UTF-8. La consola de Windows usa
     * por defecto una pagina de
     * codigos antigua, asi que sin
     * esto un "Error en linea 1" se
     * lee "l+-nea" y quien esta
     * aprendiendo no entiende nada.
     */

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
}

int console_stderr_es_terminal(void) {

    /*
     * isatty no es C11, es del sistema
     * operativo, y por eso la llamada
     * esta aqui dentro y no repartida
     * por el proyecto. Windows la
     * llama _isatty.
     */

#ifdef _WIN32
    return _isatty(_fileno(stderr)) != 0;
#else
    return isatty(fileno(stderr)) != 0;
#endif
}
