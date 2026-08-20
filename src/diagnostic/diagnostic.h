#ifndef TZLANG_DIAGNOSTIC_H
#define TZLANG_DIAGNOSTIC_H

/*
 * ==========================
 * NOTAS DE DIAGNOSTICO
 * ==========================
 *
 * Capa ANADIDA sobre los mensajes de
 * error que ya existen. No los
 * sustituye ni los reformatea: el
 * diagnostico tecnico sale igual que
 * siempre, y al final del todo se
 * anade una frase.
 *
 * El flujo es:
 *
 *   error tecnico  (lo de siempre)
 *        v
 *   categoria      (se registra aqui)
 *        v
 *   nota           (una frase fija)
 *
 * Quien detecta el error REGISTRA la
 * categoria. Quien termina el
 * programa IMPRIME la nota. Asi sale
 * una sola nota por ejecucion, al
 * final, pase por donde pase el
 * error.
 */

typedef enum {

    DIAG_NINGUNA = 0,

    /* Lexico */
    DIAG_LEXICO,

    /* Sintaxis */
    DIAG_PARENTESIS_FALTANTE,
    DIAG_LLAVE_FALTANTE,
    DIAG_CORCHETE_FALTANTE,
    DIAG_SIMBOLO_FALTANTE,
    DIAG_PALABRA_CLAVE_FALTANTE,
    DIAG_TOKEN_INESPERADO,
    DIAG_SINTAXIS,

    /* Ejecucion */
    DIAG_VARIABLE_NO_DEFINIDA,
    DIAG_TIPO,
    DIAG_INDICE,
    DIAG_CLAVE,
    DIAG_ARGUMENTO,
    DIAG_SEMANTICO

} DiagnosticCategoria;

/*
 * Registra la categoria del error que
 * se acaba de emitir. Solo se guarda
 * la PRIMERA: es la que causo todo lo
 * demas.
 */
void diagnostic_registrar(DiagnosticCategoria categoria);

/*
 * Deduce la categoria a partir del
 * texto del mensaje del parser, para
 * no tener que tocar cada uno de sus
 * puntos de error.
 */
DiagnosticCategoria diagnostic_categoria_de_mensaje(const char *mensaje);

/*
 * La frase de una categoria. Siempre
 * la misma para la misma categoria:
 * sin azar, para que los tests sean
 * deterministas.
 *
 * Devuelve NULL si no hay nota.
 */
const char *diagnostic_nota(DiagnosticCategoria categoria);

/*
 * Imprime la nota de la categoria
 * registrada, si procede. No imprime
 * nada si no se registro ninguna o si
 * las notas estan desactivadas.
 */
void diagnostic_imprimir_nota(void);

/*
 * ¿Estan activas las notas?
 *
 *   TZ_NOTAS=1   siempre
 *   TZ_NOTAS=0   nunca
 *   sin definir  solo si stderr es un
 *                terminal
 *
 * Que por defecto dependa del
 * terminal es lo que mantiene intacta
 * la salida cuando alguien redirige
 * los errores a un archivo o los
 * procesa con un script.
 */
int diagnostic_notas_activas(void);

/*
 * Vuelve al estado inicial. Existe
 * para los tests.
 */
void diagnostic_reiniciar(void);

#endif
