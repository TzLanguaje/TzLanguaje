# Canales de distribución pendientes

Lo que hay aquí está preparado, pero
**no publicado**. El README solo
anuncia lo que de verdad funciona hoy:
los instaladores nativos y los
binarios de cada release.

## Homebrew — `homebrew/tzlang.rb`

Necesita un repositorio aparte llamado
`TzLanguaje/homebrew-tzlang`, con la
fórmula en `Formula/tzlang.rb`.

Cuando exista, los usuarios instalarán
con:

    brew install tzlanguaje/tzlang/tzlang

Compila desde el código, así que la
misma fórmula sirve para macOS (Intel
y Apple Silicon) y para Linux.

## Scoop — `scoop/tzlang.json`

Necesita `TzLanguaje/scoop-tzlang`, con
el manifiesto en `bucket/tzlang.json`.

    scoop bucket add tzlang https://github.com/TzLanguaje/scoop-tzlang
    scoop install tzlang

Lleva bloque `autoupdate`, así que
`scoop update` encuentra las versiones
nuevas sin editarlo a mano.

## npm — `npm/`

Aparcado. Ver `npm/LEEME.md`.

## Al activar cualquiera de ellos

Volver a añadir su sección al README,
en «Otras formas de instalar».
