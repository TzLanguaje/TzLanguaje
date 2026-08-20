#!/bin/sh
#
# ==========================
# Genera los iconos de TzLang
# ==========================
#
#   sh packaging/icono/generar-iconos.sh
#
# A partir de tzlang-original.png produce
# todo lo que necesita cada sistema:
#
#   tzlang.ico            Windows
#   tzlang-NNN.png        Linux (varios tamaños)
#
# Solo usa sips, que viene con macOS.
# El .ico se escribe a mano porque el
# formato es sencillo: una cabecera y
# los PNG dentro.

set -eu

AQUI="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
ORIGEN="$AQUI/tzlang-original.png"

[ -f "$ORIGEN" ] || { echo "Falta $ORIGEN" >&2; exit 1; }

TAMANOS="16 24 32 48 64 128 256 512"

# ==========================
# RECORTAR EL MARGEN
# ==========================
#
# El original tiene mucho fondo
# alrededor del dibujo: el pajaro ocupa
# solo el 59% del ancho. A 16x16 eso
# seria un cuadrado casi negro.
#
# Se recorta a un cuadrado centrado que
# deja al dibujo ocupando el marco, con
# un margen pequeño para que no toque
# los bordes.

RECORTE=1030

echo "Recortando el margen sobrante..."

sips -c "$RECORTE" "$RECORTE" "$ORIGEN" --out "$AQUI/.recortado.png" >/dev/null 2>&1

echo "Redimensionando..."

for t in $TAMANOS; do
    sips -z "$t" "$t" "$AQUI/.recortado.png" --out "$AQUI/tzlang-$t.png" >/dev/null 2>&1
    printf '   %sx%s\n' "$t" "$t"
done

rm -f "$AQUI/.recortado.png"

# ==========================
# .ico para Windows
# ==========================
#
# Windows Vista en adelante acepta
# entradas comprimidas como PNG dentro
# del .ico, asi que basta con
# empaquetarlas.

echo "Empaquetando tzlang.ico..."

python3 - "$AQUI" <<'PYTHON'
import struct, sys, os

carpeta = sys.argv[1]
tamanos = [16, 24, 32, 48, 64, 128, 256]

imagenes = []

for t in tamanos:
    ruta = os.path.join(carpeta, "tzlang-%d.png" % t)
    with open(ruta, "rb") as f:
        imagenes.append((t, f.read()))

# Cabecera ICONDIR: reservado, tipo 1 (icono), cuantas imagenes
salida = struct.pack("<HHH", 0, 1, len(imagenes))

# Cada entrada del directorio ocupa 16 bytes
desplazamiento = 6 + 16 * len(imagenes)

for t, datos in imagenes:
    # 256 se codifica como 0
    ancho = 0 if t >= 256 else t
    salida += struct.pack(
        "<BBBBHHII",
        ancho, ancho,   # ancho y alto
        0,              # colores de la paleta
        0,              # reservado
        1,              # planos
        32,             # bits por pixel
        len(datos),
        desplazamiento,
    )
    desplazamiento += len(datos)

for _, datos in imagenes:
    salida += datos

destino = os.path.join(carpeta, "tzlang.ico")

with open(destino, "wb") as f:
    f.write(salida)

print("   %s (%d imagenes, %.0f KB)" % (
    os.path.basename(destino), len(imagenes), len(salida) / 1024))
PYTHON

# ==========================
# .icns para macOS
# ==========================
#
# iconutil exige una carpeta .iconset
# con nombres exactos, incluidas las
# versiones @2x para pantallas Retina.

echo "Empaquetando tzlang.icns..."

SET="$AQUI/.tzlang.iconset"
rm -rf "$SET"
mkdir -p "$SET"

sips -c "$RECORTE" "$RECORTE" "$ORIGEN" --out "$AQUI/.recortado.png" >/dev/null 2>&1

for par in "16 icon_16x16" "32 icon_16x16@2x" "32 icon_32x32" "64 icon_32x32@2x" \
           "128 icon_128x128" "256 icon_128x128@2x" "256 icon_256x256" \
           "512 icon_256x256@2x" "512 icon_512x512" "1024 icon_512x512@2x"; do
    px="${par% *}"
    nombre="${par#* }"
    sips -z "$px" "$px" "$AQUI/.recortado.png" --out "$SET/$nombre.png" >/dev/null 2>&1
done

iconutil -c icns "$SET" -o "$AQUI/tzlang.icns"

rm -rf "$SET" "$AQUI/.recortado.png"

printf '   tzlang.icns (%s KB)\n' "$(( $(wc -c < "$AQUI/tzlang.icns") / 1024 ))"

echo "Listo."
