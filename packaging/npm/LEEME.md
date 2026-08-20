# Publicación en npm — pendiente

Todo lo necesario está aquí y probado,
pero **está desactivado a propósito**.
No se ha publicado nada en npm todavía.

## Qué hay

- `build-npm.sh` — ensambla los cinco
  paquetes desde los artefactos de un
  release.
- `plantilla/bin/tz.js` — el lanzador
  que localiza el binario de la
  plataforma y lo ejecuta.

Son cinco paquetes, siguiendo el
patrón de esbuild:

| Paquete | Contenido |
|---|---|
| `tzlang` | el lanzador, sin binario |
| `tzlang-darwin` | binario universal de macOS |
| `tzlang-linux-x64` | |
| `tzlang-linux-arm64` | |
| `tzlang-win32-x64` | |

Los cuatro de plataforma declaran `os`
y `cpu`, así que npm instala solo el
que corresponde: unos 90 KB por
usuario, no 360.

## Cómo activarlo

1. **Token de npm.** Un *Classic
   Token → Automation*, o uno granular
   con *bypass 2FA* y alcance **All
   packages** (los paquetes aún no
   existen, así que no se pueden
   seleccionar de una lista).

2. **Secreto.** `Settings › Secrets and
   variables › Actions` → `NPM_TOKEN`.

3. **Interruptor.** En la pestaña
   *Variables* de esa misma página:

   ```
   PUBLICAR_EN_NPM = si
   ```

4. Etiquetar una versión nueva. El job
   `npm` del workflow se encarga del
   resto.

No hace falta tocar ningún archivo.

## Antes de publicar

- Comprobar que el nombre `tzlang`
  sigue libre en npmjs.com.
- Volver a añadir la sección de npm al
  README: se quitó para no anunciar un
  método de instalación que todavía no
  funciona.

## Estado

El intento del 20 de agosto de 2026
falló con `403 Forbidden` por un token
sin permiso para saltarse el 2FA.
**No se publicó ningún paquete**, así
que el registro está limpio y
cualquier versión sigue disponible.

Desde entonces el workflow comprueba
`npm whoami` antes de publicar nada,
para no dejar paquetes a medias si la
credencial falla.
