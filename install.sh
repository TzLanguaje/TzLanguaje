#!/bin/sh
#
# ==========================
# Instalador de TzLang (macOS / Linux)
# ==========================
#
#   curl -fsSL https://raw.githubusercontent.com/TzLanguaje/TzLanguaje/main/install.sh | sh
#
# Descarga el binario del ultimo
# release, lo verifica contra
# SHA256SUMS.txt y lo deja en el PATH
# con el nombre 'tz'.
#
# Variables que se pueden ajustar:
#
#   TZ_VERSION   version concreta (ej. v0.3.3). Por defecto, la ultima.
#   TZ_PREFIX    donde instalar. Por defecto ~/.local
#   TZ_REPO      owner/repo de GitHub
#
# Ejemplos:
#
#   TZ_PREFIX=/usr/local sh install.sh
#   TZ_VERSION=v0.3.3 sh install.sh

set -eu

REPO="${TZ_REPO:-TzLanguaje/TzLanguaje}"
PREFIX="${TZ_PREFIX:-$HOME/.local}"
BIN_DIR="$PREFIX/bin"

rojo()  { printf '\033[31m%s\0.3.3m\n' "$1" >&2; }
verde() { printf '\033[32m%s\0.3.3m\n' "$1"; }
info()  { printf '%s\n' "$1"; }

morir() { rojo "Error: $1"; exit 1; }

# ==========================
# HERRAMIENTAS
# ==========================

if command -v curl >/dev/null 2>&1; then
    bajar()  { curl -fsSL "$1" -o "$2"; }
    leer()   { curl -fsSL "$1"; }
    # Para poder decir POR QUE fallo:
    # 404 (no hay releases) no es lo
    # mismo que no tener internet.
    codigo() { curl -sSL -o /dev/null -w '%{http_code}' "$1" 2>/dev/null; }
elif command -v wget >/dev/null 2>&1; then
    bajar()  { wget -qO "$2" "$1"; }
    leer()   { wget -qO - "$1"; }
    codigo() { echo ""; }
else
    morir "hace falta curl o wget"
fi

command -v tar >/dev/null 2>&1 || morir "hace falta tar"

# ==========================
# PLATAFORMA
# ==========================
#
# Los nombres tienen que coincidir
# con los targets de
# .github/workflows/release.yml

so="$(uname -s)"
arco="$(uname -m)"

case "$so" in
    Darwin)
        # El binario de macOS es
        # universal: sirve para Intel
        # y para Apple Silicon.
        target="macos-universal"
        ;;
    Linux)
        case "$arco" in
            x86_64|amd64)   target="linux-x86_64"  ;;
            aarch64|arm64)  target="linux-aarch64" ;;
            *) morir "arquitectura no soportada: $arco (compila desde el codigo: make && sudo make install)" ;;
        esac
        ;;
    *)
        morir "sistema no soportado: $so (en Windows usa install.ps1)"
        ;;
esac

# ==========================
# VERSION
# ==========================

if [ -n "${TZ_VERSION:-}" ]; then
    version="$TZ_VERSION"
else
    info "Buscando la ultima version..."
    version="$(
        leer "https://api.github.com/repos/$REPO/releases/latest" |
        sed -n 's/.*"tag_name"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' |
        head -n 1
    )"
    if [ -z "$version" ]; then

        # Ojo: con 'curl | sh' la
        # variable hay que ponerla en el
        # sh, no en el curl. Por eso el
        # ejemplo va escrito asi.
        ayuda="
Para instalar una version concreta:

    curl -fsSL https://raw.githubusercontent.com/$REPO/main/install.sh | TZ_VERSION=v0.3.3 sh"

        case "$(codigo "https://api.github.com/repos/$REPO/releases/latest")" in
            404) morir "el repositorio $REPO todavia no tiene ninguna release publicada.$ayuda" ;;
            403) morir "GitHub ha limitado las consultas desde esta red. Espera unos minutos.$ayuda" ;;
            *)   morir "no se pudo consultar GitHub. Revisa tu conexion a internet.$ayuda" ;;
        esac
    fi
fi

archivo="tzlang-$version-$target.tar.gz"
base="https://github.com/$REPO/releases/download/$version"

info "TzLang $version  ($target)"

# ==========================
# DESCARGAR Y VERIFICAR
# ==========================

tmp="$(mktemp -d)"
# shellcheck disable=SC2064
trap "rm -rf '$tmp'" EXIT INT TERM

info "Descargando $archivo..."
bajar "$base/$archivo" "$tmp/$archivo" ||
    morir "no se pudo descargar $base/$archivo"

# El release publica SHA256SUMS.txt.
# Si esta y hay con que comprobarlo,
# se comprueba.

if bajar "$base/SHA256SUMS.txt" "$tmp/SHA256SUMS.txt" 2>/dev/null; then

    if command -v sha256sum >/dev/null 2>&1; then
        suma="$(sha256sum "$tmp/$archivo" | cut -d' ' -f1)"
    elif command -v shasum >/dev/null 2>&1; then
        suma="$(shasum -a 256 "$tmp/$archivo" | cut -d' ' -f1)"
    else
        suma=""
    fi

    if [ -n "$suma" ]; then
        # Los puntos del nombre se
        # escapan: en una expresion
        # regular '.' vale por
        # cualquier caracter y la
        # linea equivocada podria
        # colar.
        patron="$(printf '%s' "$archivo" | sed 's/[.]/[.]/g')"

        esperada="$(sed -n "s/^\([0-9a-f]\{64\}\)[[:space:]][[:space:]]*[*]\{0,1\}$patron\$/\1/p" "$tmp/SHA256SUMS.txt" | head -n 1)"
        if [ -z "$esperada" ]; then
            info "Aviso: $archivo no aparece en SHA256SUMS.txt, no se verifica."
        elif [ "$suma" != "$esperada" ]; then
            morir "el checksum no coincide. Descarga corrupta o manipulada; no se instala nada."
        else
            info "Checksum verificado."
        fi
    fi
fi

# ==========================
# INSTALAR
# ==========================

tar -xzf "$tmp/$archivo" -C "$tmp"

origen="$tmp/tzlang-$version-$target/tz"
[ -f "$origen" ] || morir "el paquete no contiene el binario 'tz'"

mkdir -p "$BIN_DIR" || morir "no se pudo crear $BIN_DIR"

# install(1) es atomico y pone los
# permisos de una vez; cp es el plan
# B para sistemas que no lo traen.

if command -v install >/dev/null 2>&1; then
    install -m 755 "$origen" "$BIN_DIR/tz" ||
        morir "no se pudo escribir en $BIN_DIR (prueba: TZ_PREFIX=\$HOME/.local sh install.sh)"
else
    cp "$origen" "$BIN_DIR/tz" && chmod 755 "$BIN_DIR/tz" ||
        morir "no se pudo escribir en $BIN_DIR"
fi

verde "TzLang instalado en $BIN_DIR/tz"

# ==========================
# PATH
# ==========================

case ":$PATH:" in
    *":$BIN_DIR:"*)
        info ""
        info "Pruebalo:  tz --version"
        ;;
    *)
        info ""
        info "$BIN_DIR no esta en tu PATH. Anade esta linea a tu"
        info "~/.zshrc o ~/.bashrc y abre una terminal nueva:"
        info ""
        info "    export PATH=\"$BIN_DIR:\$PATH\""
        info ""
        info "Mientras tanto:  $BIN_DIR/tz --version"
        ;;
esac
