#!/usr/bin/env node
"use strict";

/*
 * ==========================
 * Lanzador de TzLang para npm
 * ==========================
 *
 * El paquete 'tzlang' no contiene
 * ningun binario. Los binarios viven
 * en paquetes aparte, uno por
 * plataforma, declarados como
 * optionalDependencies con sus campos
 * 'os' y 'cpu'.
 *
 * npm instala UNICAMENTE el que
 * corresponde a la maquina. Este
 * archivo solo lo localiza y lo
 * ejecuta.
 *
 * Es el mismo enfoque que usan
 * esbuild o biome, y evita el
 * postinstall que descarga de
 * internet: eso falla con
 * --ignore-scripts, detras de un
 * proxy y sin conexion.
 */

const { spawnSync } = require("child_process");
const path = require("path");
const fs = require("fs");

/*
 * Que paquete toca en esta maquina.
 * Los nombres coinciden con los de
 * optionalDependencies del
 * package.json principal.
 */

function paqueteDePlataforma() {

    const so = process.platform;
    const arco = process.arch;

    /* El binario de macOS es universal:
       sirve para Intel y Apple Silicon. */
    if (so === "darwin") {
        return "tzlang-darwin";
    }

    if (so === "linux" && arco === "x64") {
        return "tzlang-linux-x64";
    }

    if (so === "linux" && arco === "arm64") {
        return "tzlang-linux-arm64";
    }

    if (so === "win32" && arco === "x64") {
        return "tzlang-win32-x64";
    }

    return null;
}

const paquete = paqueteDePlataforma();

if (paquete === null) {
    process.stderr.write(
        "TzLang no tiene binario para " +
        process.platform + "-" + process.arch + ".\n\n" +
        "Puedes compilarlo desde el codigo:\n" +
        "  https://github.com/TzLanguaje/TzLanguaje\n"
    );
    process.exit(1);
}

const ejecutable =
    process.platform === "win32" ? "tz.exe" : "tz";

let ruta = null;

try {
    /*
     * Se resuelve el package.json y no
     * el binario: asi funciona igual
     * con npm, pnpm y yarn, que
     * colocan los paquetes en sitios
     * distintos.
     */
    ruta = path.join(
        path.dirname(require.resolve(paquete + "/package.json")),
        "bin",
        ejecutable
    );
} catch (e) {
    /*
     * Plan B: buscarlo al lado de este
     * mismo paquete. Cuando 'tzlang'
     * esta enlazado simbolicamente
     * (pnpm, un monorepo, npm link) la
     * resolucion normal sale del arbol
     * del proyecto y no lo encuentra,
     * aunque este instalado.
     */
    const hermano = path.join(
        __dirname, "..", "..", paquete, "bin", ejecutable
    );

    if (fs.existsSync(hermano)) {
        ruta = hermano;
    }
}

if (ruta === null) {
    process.stderr.write(
        "No se encontro el paquete " + paquete + ".\n\n" +
        "Suele pasar cuando se instalo con --no-optional.\n" +
        "Prueba a reinstalar:\n" +
        "  npm install -g tzlang\n"
    );
    process.exit(1);
}

/*
 * stdio 'inherit' es importante por
 * dos motivos: el programa escribe
 * directamente en el terminal real, y
 * TzLang puede seguir detectando si
 * stderr es un terminal para decidir
 * si muestra las notas de
 * diagnostico.
 */

const resultado = spawnSync(
    ruta,
    process.argv.slice(2),
    { stdio: "inherit" }
);

if (resultado.error) {
    process.stderr.write(
        "No se pudo ejecutar TzLang: " +
        resultado.error.message + "\n"
    );
    process.exit(1);
}

/*
 * Se propaga el codigo de salida tal
 * cual: TzLang distingue 0, 1, 2 y 3,
 * y los scripts que lo usen deben
 * seguir viendo el mismo.
 */

process.exit(resultado.status === null ? 1 : resultado.status);
