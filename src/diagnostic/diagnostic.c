#include "diagnostic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../io/console.h"

/*
 * ==========================
 * ESTADO
 * ==========================
 *
 * El CLI ejecuta un programa y
 * termina: un unico estado global
 * basta y evita tener que pasar un
 * puntero por todo el interprete,
 * que seria justo la reescritura que
 * NO queremos.
 */

static DiagnosticCategoria categoria_actual = DIAG_NINGUNA;

void diagnostic_reiniciar(void) {
    categoria_actual = DIAG_NINGUNA;
}

void diagnostic_registrar(DiagnosticCategoria categoria) {

    /*
     * Solo la primera. Un error suele
     * arrastrar otros detras, y la
     * nota util es la del que empezo
     * todo.
     */

    if (categoria_actual == DIAG_NINGUNA) {
        categoria_actual = categoria;
    }
}

/*
 * ==========================
 * MENSAJE -> CATEGORIA
 * ==========================
 *
 * El parser ya centraliza sus errores
 * en dos funciones que reciben el
 * texto. Deducir la categoria de ese
 * texto evita anadir un parametro a
 * cada una de sus llamadas, que es
 * donde estaria el riesgo de romper
 * algo.
 */

DiagnosticCategoria diagnostic_categoria_de_mensaje(const char *mensaje) {

    if (mensaje == NULL) {
        return DIAG_SINTAXIS;
    }

    /*
     * El simbolo concreto manda sobre
     * la forma de la frase: "Se
     * esperaba ')'" es, antes que
     * nada, un parentesis que falta.
     */

    if (strstr(mensaje, "')'") != NULL ||
        strstr(mensaje, "'('") != NULL) {
        return DIAG_PARENTESIS_FALTANTE;
    }

    if (strstr(mensaje, "'}'") != NULL ||
        strstr(mensaje, "'{'") != NULL) {
        return DIAG_LLAVE_FALTANTE;
    }

    if (strstr(mensaje, "']'") != NULL ||
        strstr(mensaje, "'['") != NULL) {
        return DIAG_CORCHETE_FALTANTE;
    }

    if (strstr(mensaje, "rango permitido") != NULL) {
        return DIAG_ARGUMENTO;
    }

    /*
     * "'romper' solo puede utilizarse
     * dentro de un 'mientras'": la
     * forma es correcta, lo que falta
     * es el contexto.
     */

    if (strstr(mensaje, "solo puede utilizarse") != NULL) {
        return DIAG_PALABRA_CLAVE_FUERA_DE_LUGAR;
    }

    /*
     * "Se esperaba 'fin'" y "Se
     * esperaba ':'" no son lo mismo:
     * lo primero es una palabra del
     * lenguaje, lo segundo un signo.
     * Se distinguen mirando si lo
     * entrecomillado empieza por
     * letra.
     */

    {
        const char *esperado = strstr(mensaje, "Se esperaba '");

        if (esperado != NULL) {

            char primero = esperado[13];

            if ((primero >= 'a' && primero <= 'z') ||
                (primero >= 'A' && primero <= 'Z')) {
                return DIAG_PALABRA_CLAVE_FALTANTE;
            }

            return DIAG_SIMBOLO_FALTANTE;
        }
    }

    if (strstr(mensaje, "Se esperaba") != NULL) {
        return DIAG_SIMBOLO_FALTANTE;
    }

    if (strstr(mensaje, "desconocid") != NULL ||
        strstr(mensaje, "Instrucción") != NULL) {
        return DIAG_TOKEN_INESPERADO;
    }

    return DIAG_SINTAXIS;
}

/*
 * ==========================
 * LAS FRASES
 * ==========================
 *
 * Una fija por categoria. Nada de
 * azar: el mismo error da siempre la
 * misma nota, y los tests no pueden
 * volverse inestables.
 *
 * Dos lineas: la primera dice QUE
 * paso en una frase corta, la segunda
 * es la que da personalidad.
 */

const char *diagnostic_nota(DiagnosticCategoria categoria) {

    switch (categoria) {

    case DIAG_LEXICO:
        return
            "Un carácter que el lenguaje no reconoce.\n"
            "\n"
            "Basta uno para que nada de lo demás llegue a leerse.";

    case DIAG_TEXTO_SIN_CERRAR:
        return
            "Un texto quedó sin cerrar.\n"
            "\n"
            "Una comilla que falta, y el resto del archivo cambia de significado.";

    case DIAG_PARENTESIS_FALTANTE:
        return
            "Falta un paréntesis.\n"
            "\n"
            "A veces, los errores más grandes empiezan con algo así de pequeño.";

    case DIAG_LLAVE_FALTANTE:
        return
            "Una llave quedó sin cerrar.\n"
            "\n"
            "Todo lo que se abre espera, en algún momento, ser cerrado.";

    case DIAG_CORCHETE_FALTANTE:
        return
            "Un corchete quedó abierto.\n"
            "\n"
            "Un solo carácter, y el resto del programa deja de tener forma.";

    case DIAG_SIMBOLO_FALTANTE:
        return
            "Falta un pequeño símbolo.\n"
            "\n"
            "A veces, los errores más grandes comienzan con algo tan pequeño.";

    case DIAG_PALABRA_CLAVE_FALTANTE:
        return
            "Falta una palabra del lenguaje.\n"
            "\n"
            "Lo que se abre pide cerrarse; si no, nadie sabe dónde termina.";

    case DIAG_PALABRA_CLAVE_FUERA_DE_LUGAR:
        return
            "La palabra está bien escrita, pero no en este lugar.\n"
            "\n"
            "Cada instrucción necesita su contexto para significar algo.";

    case DIAG_TOKEN_INESPERADO:
        return
            "Algo apareció donde no se esperaba.\n"
            "\n"
            "El lenguaje entendió cada palabra; no el orden en que llegaron.";

    case DIAG_SINTAXIS:
        return
            "La forma no es la que el lenguaje espera.\n"
            "\n"
            "Casi. Ese pequeño detalle tenía otros planes.";

    case DIAG_VARIABLE_NO_DEFINIDA:
        return
            "Ese nombre todavía no existe.\n"
            "\n"
            "Una letra de diferencia basta para nombrar algo que nunca se creó.";

    case DIAG_FUNCION_NO_DEFINIDA:
        return
            "Esa función no está definida.\n"
            "\n"
            "Quizá falte crearla, o el nombre no sea exactamente el mismo.";

    case DIAG_DIVISION_CERO:
        return
            "Dividir entre cero no da ningún número.\n"
            "\n"
            "Hay preguntas que la aritmética se niega a responder.";

    case DIAG_DESBORDAMIENTO:
        return
            "El número creció más de lo que cabe.\n"
            "\n"
            "Todo espacio tiene un borde, aunque tarde en aparecer.";

    case DIAG_TIPO:
        return
            "Los tipos no encajan.\n"
            "\n"
            "Cada valor sabe lo que es. La operación pedía otra cosa.";

    case DIAG_INDICE:
        return
            "La posición pedida no existe.\n"
            "\n"
            "Contar desde cero: un desplazamiento diminuto, un final distinto.";

    case DIAG_CLAVE:
        return
            "Esa clave no está en el diccionario.\n"
            "\n"
            "Buscar bien en el sitio equivocado sigue sin encontrar nada.";

    case DIAG_ARGUMENTO:
        return
            "Un argumento no es lo que la función necesitaba.\n"
            "\n"
            "Un pequeño desajuste. Una gran interrupción.";

    case DIAG_SEMANTICO:
        return
            "El programa se leyó bien, pero no pudo llegar al final.\n"
            "\n"
            "Ser válido y funcionar no son siempre la misma cosa.";

    case DIAG_NINGUNA:
    default:
        return NULL;
    }
}

/*
 * ==========================
 * ¿SE IMPRIME O NO?
 * ==========================
 *
 * Por defecto, solo cuando stderr es
 * un terminal, igual que hacen los
 * compiladores con el color.
 *
 * Esto NO es un capricho: mantiene
 * byte a byte la salida que ya
 * esperan los scripts y los tests que
 * redirigen stderr a un archivo. La
 * personalidad es para quien esta
 * mirando la pantalla.
 *
 * TZ_NOTAS fuerza el comportamiento
 * en cualquiera de los dos sentidos.
 */

int diagnostic_notas_activas(void) {

    const char *ajuste = getenv("TZ_NOTAS");

    if (ajuste != NULL && ajuste[0] != '\0') {

        if (strcmp(ajuste, "0") == 0) {
            return 0;
        }

        return 1;
    }

    return console_stderr_es_terminal();
}

void diagnostic_imprimir_nota(void) {

    const char *nota;

    if (!diagnostic_notas_activas()) {
        return;
    }

    nota = diagnostic_nota(categoria_actual);

    if (nota == NULL) {
        return;
    }

    /*
     * Linea en blanco de separacion:
     * la nota no se mezcla con el
     * diagnostico tecnico.
     */

    fprintf(stderr, "\n%s\n", nota);
}
