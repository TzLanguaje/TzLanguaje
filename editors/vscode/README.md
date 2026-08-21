# TzLang para VS Code

Pone el icono de TzLang a los archivos `.tz` en el explorador de VS Code, y le enseña al editor cómo tratarlos: comentarios con `//`, paréntesis y comillas que se cierran solos, y la sangría que entra sola después de `si`, `mientras`, `para cada` y `funcion`, y sale sola con `fin`.

TzLang es un lenguaje de programación educativo en español. El lenguaje vive en
[github.com/TzLanguaje/TzLanguaje](https://github.com/TzLanguaje/TzLanguaje).

## Por qué hace falta una extensión

Los instaladores de TzLang registran el icono de los `.tz` en el sistema, y el
explorador de archivos de Windows, Linux y macOS lo respeta.

VS Code no. Sus iconos salen de su propio **tema de iconos**, que no sabe nada
de las asociaciones del sistema operativo, y ningún tema conoce la extensión
`.tz`. La única forma de que aparezca es que una extensión lo declare, que es
justo lo que hace esta.

## Instalar

Hay que instalarla **en cada máquina**. VS Code no sincroniza las extensiones
que vienen de un `.vsix`: Settings Sync solo propaga las del Marketplace.

Cada release publica el `.vsix` junto a los demás archivos. En **Linux y macOS**:

```bash
curl -fsSLO https://github.com/TzLanguaje/TzLanguaje/releases/latest/download/tzlang-0.4.1.vsix
code --install-extension tzlang-0.4.1.vsix
```

En **Windows**, con PowerShell:

```powershell
irm https://github.com/TzLanguaje/TzLanguaje/releases/latest/download/tzlang-0.4.1.vsix -OutFile tzlang.vsix
code --install-extension tzlang.vsix
```

O construyéndolo desde el repositorio, que es lo mismo:

```bash
sh editors/vscode/construir-vsix.sh
```

Después de instalarla hay que **recargar la ventana** para ver el cambio:
`Ctrl+Shift+P` (`Cmd+Shift+P` en Mac) → *Developer: Reload Window*.

## Si el icono sigue sin aparecer

Casi siempre es el tema de iconos. VS Code solo usa el icono de la extensión
cuando el tema no tiene uno propio para ese archivo **y** no ha desactivado los
iconos de lenguaje.

- Si tu tema define un icono genérico para archivos desconocidos, ese gana.
  Prueba con otro tema para descartarlo: `Ctrl+Shift+P` → *File Icon Theme*.
- El tema **Minimal** no dibuja iconos de archivo en absoluto.
- Un tema puede llevar `"showLanguageModeIcons": false` en su JSON, y entonces
  ignora a propósito los iconos de todas las extensiones, no solo el de esta.

## Lo que todavía no hace

**No colorea la sintaxis.** Eso necesita una gramática TextMate, que es otro
trabajo y no está hecho. Los `.tz` se ven en un solo color, igual que antes,
pero ya con su icono y con el editor sabiendo dónde van las sangrías.
