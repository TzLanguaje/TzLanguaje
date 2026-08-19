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

#ifdef _WIN32
#include <windows.h>
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
