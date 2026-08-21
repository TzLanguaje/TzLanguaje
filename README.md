<h1 align="center">TzLang</h1>

<p align="center">
  <strong>Un lenguaje de programación educativo en español, construido desde cero en C11.</strong>
</p>

<p align="center">
  <a href="https://github.com/TzLanguaje/TzLanguaje/actions/workflows/ci.yml"><img src="https://github.com/TzLanguaje/TzLanguaje/actions/workflows/ci.yml/badge.svg" alt="CI"></a>
  <a href="https://github.com/TzLanguaje/TzLanguaje/releases/latest"><img src="https://img.shields.io/github/v/release/TzLanguaje/TzLanguaje?label=versi%C3%B3n&amp;color=blue" alt="Última versión"></a>
  <img src="https://img.shields.io/badge/C-C11-blue" alt="C11">
  <img src="https://img.shields.io/badge/tests-173%20passed-success" alt="173 tests">
  <img src="https://img.shields.io/badge/licencia-MIT-green" alt="Licencia MIT">
  <img src="https://img.shields.io/badge/plataformas-macOS%20%7C%20Linux%20%7C%20Windows-lightgrey" alt="Plataformas">
</p>

<p align="center">
  <a href="https://github.com/TzLanguaje/TzLanguaje/releases/latest"><b>⬇ Descargar e instalar</b></a>
  &nbsp;·&nbsp;
  <a href="#instalación">Guía de instalación</a>
  &nbsp;·&nbsp;
  <a href="docs/language.md">Referencia del lenguaje</a>
</p>

---

## ¿Qué es TzLang?

**TzLang** es un lenguaje de programación **interpretado** con sintaxis en **español**, pensado para aprender los conceptos fundamentales de la programación sin la barrera del inglés.

Está construido **desde cero en C11**, sin dependencias externas ni generadores de parsers. Todas las piezas son propias:

| Componente | Responsabilidad |
|---|---|
| **Lexer** | Convierte el texto fuente en tokens |
| **Parser** | Analiza los tokens y construye el árbol sintáctico |
| **AST** | Representa la estructura del programa |
| **Interpreter** | Recorre el AST y ejecuta el programa |
| **Runtime** | Gestiona valores, operaciones y memoria |

Los programas se escriben en archivos con extensión `.tz` y se ejecutan con el comando `tz`.

```tz
imprimir "Hola desde TzLang"
```

```
Hola desde TzLang
```

---

## La sintaxis en español es el punto de partida

Lo que distingue a TzLang de un intérprete de juguete cualquiera es que **las comparaciones se escriben como se dicen en voz alta**. Quien está aprendiendo no necesita traducir mentalmente `>=` antes de entender qué hace su programa:

```tz
variable edad = 20

si edad es mayor o igual que 18
    imprimir "Mayor de edad"
sino
    imprimir "Menor de edad"
fin
```

```
Mayor de edad
```

Las seis formas comparativas son parte del lenguaje, no azúcar sintáctico añadido después:

| Forma en español | Equivalente simbólico |
|---|---|
| `es mayor que` | `>` |
| `es menor que` | `<` |
| `es mayor o igual que` | `>=` |
| `es menor o igual que` | `<=` |
| `es igual a` | `==` |
| `es diferente de` | `!=` |

Ambas notaciones son intercambiables y se pueden mezclar. La forma simbólica sigue disponible para quien ya la conoce o quiera acostumbrarse a ella:

```tz
variable edad = 20

imprimir edad es mayor que 18
imprimir edad > 18
```

```
verdadero
verdadero
```

Los operadores lógicos siguen la misma idea: `y`, `o` y `no`.

```tz
variable edad = 20
variable tiene_documento = verdadero

si (edad es mayor o igual que 18) y (tiene_documento)
    imprimir "Puede entrar"
fin

si no (edad es igual a 30)
    imprimir "No tiene 30"
fin
```

```
Puede entrar
No tiene 30
```

---

## Un programa completo

Este ejemplo reúne funciones, diccionarios, listas, bucles y condicionales:

```tz
// Clasificar estudiantes por su nota

funcion clasificar(nota)
    si nota es mayor o igual que 90
        retornar "Sobresaliente"
    fin
    si nota es mayor o igual que 70
        retornar "Aprobado"
    fin
    retornar "Suspenso"
fin

variable estudiantes = [
    {"nombre": "Ana", "nota": 95},
    {"nombre": "Carlos", "nota": 72},
    {"nombre": "Lucia", "nota": 48}
]

variable aprobados = 0

para cada estudiante en estudiantes

    variable nombre = estudiante["nombre"]
    variable nota = estudiante["nota"]
    variable resultado = clasificar(nota)

    imprimir nombre + ": " + resultado

    si nota es mayor o igual que 70
        aprobados = aprobados + 1
    fin
fin

imprimir "Aprobados: " + texto(aprobados) + " de " + texto(largo(estudiantes))
```

```
Ana: Sobresaliente
Carlos: Aprobado
Lucia: Suspenso
Aprobados: 2 de 3
```

---

## Instalación

No hace falta compilar nada ni instalar dependencias. Descargas un archivo, lo abres, y listo.

**Todos los instaladores están aquí: [página de descargas](https://github.com/TzLanguaje/TzLanguaje/releases/latest)**

Baja hasta **Assets** y elige el archivo que corresponda a tu ordenador:

| Si usas… | Descarga este archivo |
|---|---|
| **Windows** | `TzLang-vX.Y.Z-windows-x86_64-setup.exe` |
| **Mac** (cualquiera, Intel o M1/M2/M3) | `TzLang-vX.Y.Z-macos.pkg` |
| **Ubuntu, Debian, Mint** | `tzlang_X.Y.Z_amd64.deb` |
| **Fedora, RHEL, openSUSE** | `tzlang-X.Y.Z-1.x86_64.rpm` |

> En una Raspberry Pi o un portátil ARM, cambia `amd64` por `arm64` (en el `.deb`) o `x86_64` por `aarch64` (en el `.rpm`).

---

### Windows

1. Descarga el archivo `…-setup.exe` y haz **doble clic**.
2. Windows mostrará un aviso azul que dice *«Windows protegió su PC»*. Es normal: aparece con cualquier programa sin firma de pago. Pulsa **Más información** y luego **Ejecutar de todas formas**.
3. Sigue el asistente pulsando **Siguiente**. Deja marcadas las dos casillas:
   - **Añadir TzLang al PATH** — es la que hace que la orden `tz` funcione.
   - **Asociar los archivos .tz** — te permite ejecutar programas con doble clic.
4. Pulsa **Instalar** y luego **Finalizar**.

No pide contraseña de administrador. Para desinstalarlo, búscalo en **Agregar o quitar programas**.

### macOS

1. Descarga el archivo `.pkg`.
2. **No lo abras con doble clic la primera vez.** Haz **clic derecho** sobre él y elige **Abrir**. Luego, en el aviso que sale, pulsa **Abrir** otra vez.

   Esto hace falta porque el instalador no está firmado con una cuenta de Apple Developer (cuesta 99 $ al año). Con doble clic normal, macOS lo bloquea sin darte opción.
3. Sigue el asistente: **Continuar**, **Aceptar** la licencia, **Instalar**.
4. Te pedirá tu contraseña de usuario. Es normal: instala en una carpeta del sistema.

### Ubuntu, Debian, Mint

Haz **doble clic** sobre el archivo `.deb` y pulsa **Instalar** en el centro de software.

Si prefieres la terminal:

```bash
sudo apt install ./tzlang_X.Y.Z_amd64.deb
```

Para desinstalarlo: `sudo apt remove tzlang`

### Fedora, RHEL, openSUSE

Haz **doble clic** sobre el archivo `.rpm`, o desde la terminal:

```bash
sudo dnf install ./tzlang-X.Y.Z-1.x86_64.rpm
```

Para desinstalarlo: `sudo dnf remove tzlang`

---

### Comprueba que funciona

Abre una terminal **nueva** — importante, una que hayas abierto *después* de instalar:

- **Windows**: busca *PowerShell* en el menú de inicio.
- **Mac**: busca *Terminal* en Spotlight (⌘ + espacio).
- **Linux**: Ctrl + Alt + T.

Escribe:

```bash
tz --version
```

Si responde con el nombre y el número de versión, por ejemplo `TzLang 0.4.0`, ya está instalado.

Si en cambio dice *«orden no encontrada»* o *«no se reconoce como un comando»*, casi siempre es una de dos cosas: la terminal estaba abierta desde antes de instalar (ciérrala y abre otra), o en Windows desmarcaste la casilla del PATH (vuelve a pasar el instalador y déjala marcada).

### Tu primer programa

Crea un archivo llamado `hola.tz` con esta línea dentro:

```tz
imprimir "Hola desde TzLang"
```

Y ejecútalo desde la terminal, en la carpeta donde lo guardaste:

```bash
tz hola.tz
```

```
Hola desde TzLang
```

---

## Otras formas de instalar

Para quien prefiera la terminal, o quiera automatizar la instalación.

### Instalador de una línea

**macOS y Linux**

```bash
curl -fsSL https://raw.githubusercontent.com/TzLanguaje/TzLanguaje/main/install.sh | sh
```

Verifica el checksum SHA-256 e instala en `~/.local/bin/tz`, sin permisos de administrador. Se puede ajustar con `TZ_PREFIX` y `TZ_VERSION`:

```bash
TZ_PREFIX=/usr/local sh install.sh
TZ_VERSION=v0.4.0    sh install.sh
```

**Windows**, en PowerShell:

```powershell
irm https://raw.githubusercontent.com/TzLanguaje/TzLanguaje/main/install.ps1 | iex
```

### Gestores de paquetes

**macOS y Linux**, con Homebrew:

```bash
brew install tzlanguaje/tzlang/tzlang
```

Compila desde el código, así que sirve igual en Intel, Apple Silicon y Linux.

**Windows**, con Scoop:

```powershell
scoop bucket add tzlang https://github.com/TzLanguaje/scoop-tzlang
scoop install tzlang
```

Descarga el binario y verifica su checksum SHA-256.

### Binario suelto

Para meterlo en un contenedor o llevarlo en un USB, cada release publica también archivos comprimidos con el ejecutable pelado, y un `SHA256SUMS.txt` para verificarlos:

| Sistema | Archivo |
|---|---|
| macOS (Intel y Apple Silicon) | `tzlang-vX.Y.Z-macos-universal.tar.gz` |
| Linux x86-64 | `tzlang-vX.Y.Z-linux-x86_64.tar.gz` |
| Linux ARM64 | `tzlang-vX.Y.Z-linux-aarch64.tar.gz` |
| Windows x86-64 | `tzlang-vX.Y.Z-windows-x86_64.zip` |

Los binarios de Linux están enlazados estáticamente: funcionan en cualquier distribución sin depender de la versión de `glibc`.

---

## Compilar desde el código

No hace falta ninguna biblioteca externa: basta un compilador de C11.

### Con CMake (los tres sistemas)

Es la vía recomendada, y la única que funciona en Windows.

```bash
git clone https://github.com/TzLanguaje/TzLanguaje.git
cd TzLanguaje
cmake -B build-cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake
```

El ejecutable queda en `build-cmake/tz` (`build-cmake\Release\tz.exe` en Windows). Para instalarlo:

```bash
cmake --install build-cmake --prefix ~/.local
```

En Windows sirve Visual Studio 2019 16.8 o posterior (por `/std:c11`), o MinGW-w64.

### Con Make (macOS y Linux)

Es el flujo de desarrollo del proyecto: `make test`, `make debug` y `make asan` viven aquí.

```bash
make
./build/tzc examples/hola.tz
```

Para instalar el comando `tz`:

```bash
sudo make install
```

Esto copia `build/tzc` a `/usr/local/bin/tz`. El destino se cambia con `PREFIX`, lo que además evita necesitar privilegios de administrador:

```bash
make PREFIX=$HOME/.local install
```

Para desinstalarlo:

```bash
sudo make uninstall
```

> El `Makefile` nunca invoca `sudo` por su cuenta: eres tú quien decide si hace falta según el `PREFIX` elegido.

En macOS, las herramientas de línea de órdenes se instalan con `xcode-select --install`.

---

## Uso de la línea de órdenes

```bash
tz programa.tz          # ejecutar un programa
tz --help               # mostrar la ayuda   (o -h)
tz --version            # mostrar la versión (o -v)
```

```
TzLang 0.4.0
```

Solo se aceptan archivos con extensión `.tz`.

### Códigos de salida

Están separados en familias para que un script pueda distinguir **qué** falló:

| Código | Significado |
|---|---|
| `0` | Ejecución correcta |
| `1` | Error de argumentos u opciones |
| `2` | Error de archivo o extensión |
| `3` | Error de lexer, parser o interpreter |

---

## El lenguaje

### Variables

Se declaran con `variable` y se reasignan directamente por su nombre:

```tz
variable nombre = "Carlos"
variable edad = 20

edad = edad + 1
imprimir edad
```

```
21
```

Los comentarios empiezan por `//`.

### Tipos de datos

TzLang tiene siete tipos, y `tipo()` devuelve el nombre de cada uno en español:

```tz
imprimir tipo(42)
imprimir tipo(3.14)
imprimir tipo("Hola")
imprimir tipo(verdadero)
imprimir tipo(nulo)
imprimir tipo([1, 2, 3])
imprimir tipo({"a": 1})
```

```
numero
decimal
texto
booleano
nulo
lista
diccionario
```

Los booleanos son `verdadero` y `falso`, y la ausencia de valor es `nulo`.

Dentro de un texto se pueden usar cuatro secuencias de escape:

| Escape | Significado |
|---|---|
| `\n` | Salto de línea |
| `\t` | Tabulador |
| `\"` | Comilla doble |
| `\\` | Barra invertida |

```tz
imprimir "Dice \"hola\""
imprimir "uno\ndos"
```

> **Nombres reservados.** `y`, `o`, `no`, `si`, `mientras` y el resto de palabras del lenguaje no pueden usarse como nombres de variable. `y` es tentador para una coordenada, pero es el operador lógico. Si lo intentas, el error te lo dice por su nombre.

### Operadores aritméticos

```tz
imprimir 7 + 3
imprimir 7 - 3
imprimir 7 * 3
imprimir 7 / 3
imprimir 7 % 3
imprimir 7.0 / 2
imprimir -5
imprimir 2 + 3 * 4
imprimir (2 + 3) * 4
```

```
10
4
21
2
3.5
-5
14
20
```

La división entre dos `numero` trunca hacia cero (`7 / 3` da `2`); si algún operando es `decimal`, el resultado es `decimal`. El operador `+` también concatena textos.

### Condicionales

```tz
variable nota = 85

si nota es mayor o igual que 90
    imprimir "Sobresaliente"
sino
    imprimir "Puede mejorar"
fin
```

Para encadenar condiciones, `sino si`. Toda la cadena se cierra con **un solo** `fin`:

```tz
si nota es mayor o igual que 90
    imprimir "Sobresaliente"
sino si nota es mayor o igual que 70
    imprimir "Aprobado"
sino si nota es mayor o igual que 50
    imprimir "Justo"
sino
    imprimir "Puede mejorar"
fin
```

Los valores `0`, `""`, `[]`, `{}`, `falso` y `nulo` se consideran falsos; el resto, verdaderos.

### Bucles

`mientras` repite mientras la condición se cumpla:

```tz
variable i = 1

mientras i es menor o igual que 3
    imprimir i
    i = i + 1
fin
```

```
1
2
3
```

`para cada` recorre listas y diccionarios:

```tz
variable frutas = ["manzana", "pera", "uva"]

para cada fruta en frutas
    imprimir fruta
fin
```

```
manzana
pera
uva
```

Sobre un diccionario, `para cada` recorre sus **claves**.

### Romper y continuar

`romper` abandona el bucle y `continuar` salta a la siguiente vuelta:

```tz
variable n = 0

mientras verdadero
    n = n + 1
    si n es igual a 2
        continuar
    fin
    si n es mayor que 4
        romper
    fin
    imprimir n
fin
```

```
1
3
4
```

### Funciones

Se definen con `funcion` y devuelven un valor con `retornar`:

```tz
funcion sumar(a, b)
    retornar a + b
fin

imprimir sumar(10, 20)
```

```
30
```

Una función sin `retornar` devuelve `nulo`. La recursión funciona con normalidad:

```tz
funcion factorial(n)
    si n es menor o igual que 1
        retornar 1
    fin
    retornar n * factorial(n - 1)
fin

imprimir factorial(5)
```

```
120
```

El **scope es léxico**: una variable declarada dentro de una función no se ve fuera de ella, y no pisa a la global del mismo nombre.

```tz
variable mensaje = "global"

funcion prueba()
    variable mensaje = "local"
    imprimir mensaje
fin

prueba()
imprimir mensaje
```

```
local
global
```

### Listas

```tz
variable numeros = [1, 2, 3]

imprimir numeros[0]
imprimir largo(numeros)

numeros[1] = 99
agregar(numeros, 4)
imprimir numeros

eliminar(numeros, 0)
imprimir numeros
```

```
1
3
[1, 99, 3, 4]
[99, 3, 4]
```

Una lista puede contener valores de distinto tipo, incluidas otras listas:

```tz
variable mixta = [1, "texto", verdadero, nulo, [2, 3]]

imprimir mixta
imprimir mixta[4][1]
```

```
[1, "texto", verdadero, nulo, [2, 3]]
3
```

Los índices negativos cuentan desde el final, y las listas se concatenan con `+`:

```tz
variable l = [10, 20, 30]
imprimir l[-1]
imprimir [1, 2] + [3, 4]
```

```
30
[1, 2, 3, 4]
```

### Diccionarios

Las claves son de tipo `texto` y se conserva el orden de inserción:

```tz
variable persona = {
    "nombre": "Carlos",
    "edad": 20
}

imprimir persona["nombre"]

persona["edad"] = 21
persona["pais"] = "Colombia"

imprimir persona
imprimir claves(persona)
imprimir valores(persona)
```

```
Carlos
{"nombre": "Carlos", "edad": 21, "pais": "Colombia"}
["nombre", "edad", "pais"]
["Carlos", 21, "Colombia"]
```

Se pueden anidar libremente con listas:

```tz
variable usuario = {
    "datos": {"edad": 20},
    "roles": ["admin", "dev"]
}

imprimir usuario["datos"]["edad"]
imprimir usuario["roles"][0]
```

```
20
admin
```

### Copia profunda

Asignar una estructura crea una **copia independiente**, no una referencia compartida. Es una decisión deliberada: evita que un principiante modifique un valor sin darse cuenta desde otro sitio.

```tz
variable a = {"datos": {"edad": 20}}
variable b = a

b["datos"]["edad"] = 99

imprimir a["datos"]["edad"]
imprimir b["datos"]["edad"]
```

```
20
99
```

---

## Entrada del usuario

`entrada()` pide un dato por teclado. Muestra el mensaje, espera a que la persona escriba una línea y **devuelve siempre un texto**:

```tz
variable nombre = entrada("¿Cómo te llamas? ")
imprimir "Hola, " + nombre

variable edad = numero(entrada("¿Cuántos años tienes? "))
imprimir "El año que viene tendrás " + texto(edad + 1)
```

```
¿Cómo te llamas? Ana
Hola, Ana
¿Cuántos años tienes? 30
El año que viene tendrás 31
```

Para trabajar con números hay que convertir con `numero()` o `decimal()`. Si no hay nada que leer —la entrada está cerrada, o el programa corre en un servidor de integración continua— devuelve un texto vacío en lugar de fallar.

---

## Funciones incorporadas

TzLang incluye 16 funciones integradas:

| Función | Descripción | Ejemplo | Resultado |
|---|---|---|---|
| `largo(x)` | Longitud de texto, lista o diccionario | `largo("Hola")` | `4` |
| `tipo(x)` | Nombre del tipo | `tipo(3.14)` | `decimal` |
| `texto(x)` | Convierte a texto, incluidas listas y diccionarios | `texto([1,2])` | `"[1, 2]"` |
| `numero(x)` | Convierte a número entero | `numero("42")` | `42` |
| `decimal(x)` | Convierte a decimal | `decimal(7)` | `7` |
| `agregar(lista, x)` | Añade un elemento al final | `agregar(l, 4)` | — |
| `eliminar(x, k)` | Borra por índice o por clave | `eliminar(l, 0)` | — |
| `contiene(x, v)` | ¿Está dentro? Busca un trozo en un texto, un valor en una lista o una clave en un diccionario | `contiene("hola", "la")` | `verdadero` |
| `unir(lista, sep)` | Une una lista de textos | `unir(["a","b"], "-")` | `"a-b"` |
| `separar(txt, sep)` | Parte un texto en lista | `separar("a,b", ",")` | `["a", "b"]` |
| `mayusculas(txt)` | Pasa a mayúsculas | `mayusculas("hola")` | `"HOLA"` |
| `minusculas(txt)` | Pasa a minúsculas | `minusculas("HOLA")` | `"hola"` |
| `absoluto(x)` | Valor absoluto | `absoluto(-7)` | `7` |
| `redondear(x)` | Redondea a `numero` | `redondear(3.7)` | `4` |
| `claves(dic)` | Lista de claves | `claves(p)` | `["nombre"]` |
| `valores(dic)` | Lista de valores | `valores(p)` | `["Carlos"]` |
| `entrada(msg)` | Pide un dato por teclado | `entrada("Nombre: ")` | `"Ana"` |

`agregar` y `eliminar` modifican la estructura que reciben; el resto devuelven un valor nuevo.

---

## Errores

Los errores se informan en español, van a `stderr` y detienen la ejecución con código `3`:

```tz
imprimir 10 / 0
```

```
Error: división por cero.
La ejecución falló.
```

```tz
imprimir desconocida
```

```
Error: variable 'desconocida' no existe.
La ejecución falló.
```

---

---

## El icono de los archivos `.tz`

TzLang trae su propio icono, y los instaladores lo registran para que el explorador de archivos lo use en los `.tz`.

| Sistema | Estado |
|---|---|
| **Windows** | Funciona por las tres vías: el `…-setup.exe`, el instalador de PowerShell y el `.zip`. Asocian los `.tz`, les ponen el icono y avisan al Explorador para que se vea al momento. El propio `tz.exe` lleva el icono incrustado, igual que la entrada de *Agregar o quitar programas*. |
| **Linux** | Funciona con los paquetes `.deb` y `.rpm`, que declaran el tipo `text/x-tzlang` e instalan el icono en siete tamaños. |
| **macOS** | Funciona. El `.pkg` instala además `TzLang.app` en Aplicaciones, que declara el tipo de archivo y permite **ejecutar un `.tz` con doble clic**: abre el Terminal y lo corre. |

En macOS hace falta esa aplicación porque el icono de un tipo de archivo **solo** se puede declarar desde el `Info.plist` de una app: un programa de línea de órdenes no tiene dónde hacerlo. Ya que hacía falta, se aprovecha para dar el doble clic.

Los iconos se regeneran desde el original con:

```bash
sh packaging/icono/generar-iconos.sh
```

### Si en Windows los `.tz` salen en blanco

El Explorador de Windows lee la lista de asociaciones **al arrancar** y no vuelve a mirarla. Por eso puede pasar que la asociación esté bien puesta y el icono siga sin aparecer.

Lo primero, comprueba que el registro la tiene:

```powershell
reg query "HKCU\Software\Classes\.tz"
reg query "HKCU\Software\Classes\TzLang.Programa\DefaultIcon"
```

- **Si las claves están** y el icono no se ve, es que el Explorador no se ha enterado. Se le avisa reiniciándolo:

  ```powershell
  Stop-Process -Name explorer -Force
  ```

  Se cierra y vuelve a abrirse solo. Si aun así no aparece, la caché de iconos está vieja:

  ```powershell
  ie4uinit.exe -show
  ```

- **Si las claves no están**, se instaló con una versión anterior a la 0.4.1, o desmarcando la casilla *Asociar los archivos .tz*. Vuelve a pasar el instalador con la casilla marcada.

- **Si sale el icono de otro programa** (el Bloc de notas, VS Code…), es porque alguna vez abriste un `.tz` con *Abrir con* y marcaste *Usar siempre esta aplicación*. Esa elección tuya manda sobre la del instalador, y ningún programa puede deshacerla por ti: clic derecho en un `.tz` → **Abrir con** → **Elegir otra aplicación** → **TzLang** → **Siempre**.

---

## Mensajes pensados para quien aprende

Cuando aparece un símbolo que TzLang no conoce, el error no se limita a señalarlo: dice qué se escribe en su lugar.

```tz
imprimir 'hola'
```

```
Error en línea 1: el carácter ''' no forma parte de TzLang.
Los textos van entre comillas dobles: "así".
```

Está previsto para los errores que arrastra quien viene de otro lenguaje: `.` para diccionarios, `#` para comentarios, `&&` y `||` para condiciones, `!` para negar, `^` para potencias.

Las comparaciones admiten también los símbolos (`>`, `<`, `==`…), pero **la forma con palabras es la recomendada**: es el sentido del lenguaje, y permite leer un programa en voz alta y entenderlo.

---

## Notas de diagnóstico

Cuando un programa falla, TzLang añade una frase debajo del error técnico.

```tz
imprimir (1 + 2
```

```
Error del Parser en línea 2: Se esperaba ')' para cerrar la expresión.
Error construyendo AST.

Falta un paréntesis.

A veces, los errores más grandes empiezan con algo así de pequeño.
```

> **A veces los errores más grandes empiezan con algo muy pequeño.**

La nota es una capa añadida: el diagnóstico técnico no cambia ni pierde información. Cada categoría de error —símbolo que falta, variable no definida, tipos que no encajan, índice fuera de rango— tiene su propia frase, siempre la misma, sin azar.

Solo aparece cuando estás delante de un terminal. Si rediriges la salida a un archivo o la procesas con un script, TzLang emite exactamente los mismos bytes que antes:

```bash
tz programa.tz 2> errores.txt    # sin notas, salida intacta
```

Se puede forzar en cualquiera de los dos sentidos:

```bash
TZ_NOTAS=1 tz programa.tz    # siempre
TZ_NOTAS=0 tz programa.tz    # nunca
```

## Arquitectura

El intérprete procesa cada programa en una tubería de etapas bien separadas, cada una en su propio directorio dentro de `src/`:

```
  archivo .tz
       │
       ▼
   ┌────────┐
   │ Lexer  │   texto  ──►  tokens
   └────────┘
       │
       ▼
   ┌────────┐
   │ Parser │   tokens ──►  AST
   └────────┘
       │
       ▼
   ┌────────┐
   │  AST   │   estructura del programa
   └────────┘
       │
       ▼
   ┌─────────────┐
   │ Interpreter │   recorre y ejecuta
   └─────────────┘
       │
       ▼
   ┌─────────┐
   │ Runtime │   valores, operaciones, memoria
   └─────────┘
       │
       ▼
    salida
```

La memoria se gestiona de forma explícita, sin recolector de basura. Los diccionarios usan un array dinámico de pares con búsqueda lineal: para el tamaño de los programas educativos a los que apunta el proyecto, esa simplicidad prima sobre el rendimiento y mantiene el código legible para quien quiera estudiarlo.

---

## Estructura del proyecto

```
TzLang/
│
├── src/
│   ├── lexer/          lexer.c / lexer.h
│   ├── parser/         parser.c / parser.h
│   ├── ast/            ast.c / ast.h
│   ├── interpreter/    interpreter.c / interpreter.h
│   ├── runtime/        value.c / operations.c
│   ├── diagnostic/     notas de diagnóstico por categoría
│   ├── io/             file.c / console.c
│   ├── main.c          punto de entrada y CLI
│   └── version.h       número de versión
│
├── examples/           programas de ejemplo
├── education/          cinco lecciones con salida esperada
│                       y el prompt generador de ejercicios
├── docs/
│   └── language.md     referencia completa del lenguaje
│
├── tests/
│   ├── run_tests.sh              suite principal
│   └── run_education_tests.sh    suite educativa
│
├── .github/workflows/
│   ├── ci.yml          compila y prueba en los tres sistemas
│   └── release.yml     publica los binarios al etiquetar
│
├── packaging/
│   ├── icono/          el icono y su generador
│   ├── macos/          instalador .pkg con asistente
│   ├── windows/        instalador .exe con asistente (Inno Setup)
│   ├── linux/          paquetes .deb y .rpm
│   ├── npm/            paquetes de npm y su lanzador
│   ├── homebrew/       plantilla de la formula de Homebrew
│   └── scoop/          plantilla del manifiesto de Scoop
│
├── scripts/
│   └── subir-version.sh
│
├── install.sh          instalador para macOS y Linux
├── install.ps1         instalador para Windows
│
├── CMakeLists.txt      build multiplataforma
├── Makefile            build de desarrollo (Unix)
├── LICENSE
└── README.md
```

---

## Desarrollo

El `Makefile` reúne todo el flujo de trabajo:

| Orden | Qué hace |
|---|---|
| `make` | Compila `build/tzc` |
| `make test` | Compila y ejecuta la suite principal |
| `make test-education` | Valida el material de `education/` |
| `make debug` | Genera `build/tzc-debug` con `-g -O0` |
| `make asan` | Genera `build/tzc-asan` y pasa la suite con sanitizers |
| `make install` | Instala el comando `tz` |
| `make uninstall` | Desinstala el comando `tz` |
| `make clean` | Borra todo lo generado en `build/` |

El compilador se puede elegir pasando la variable `CC`, por ejemplo `make CC=clang`.

### Pruebas

La suite principal ejecuta el binario real sobre archivos `.tz` y compara la salida y el código de salida con lo esperado. No enlaza contra funciones internas de C: prueba el lenguaje tal y como lo ve un usuario.

```bash
make test
```

```
========================================
Tests:  138
Passed: 138
Failed: 0
========================================

All tests passed.
```

Las 138 pruebas cubren aritmética, desbordamiento de enteros, conversiones, textos, listas, diccionarios, indexación anidada, control de flujo, funciones, recursión, scope, errores de lexer, parser e intérprete, y el comportamiento de la CLI (BOM UTF-8, CRLF, archivos vacíos, extensiones y argumentos inválidos).

### Sanitizers

```bash
make asan
```

Compila un binario aparte con **AddressSanitizer** y **UndefinedBehaviorSanitizer**, y pasa por él la misma suite completa, sin duplicarla. Sirve para detectar use-after-free, dobles liberaciones, desbordamientos de búfer y comportamiento indefinido.

Los tres binarios conviven sin pisarse: `build/tzc`, `build/tzc-debug` y `build/tzc-asan`.

---

## Education

El directorio `education/` contiene lecciones progresivas. Cada una es un programa `.tz` acompañado de un archivo `.expected` con su salida exacta, de modo que el material didáctico se verifica automáticamente y no puede quedar desactualizado respecto al lenguaje:

```bash
make test-education
```

```
=== TzLang Education Suite ===

[PASS] 01_variables

========================================
Tests:  1
Passed: 1
Failed: 0
========================================

All education tests passed.
```

Esta suite es independiente de la principal y es más estricta: exige código de salida `0`, `stderr` vacío y coincidencia exacta de `stdout`. Ahora mismo hay **una lección**; ampliar el temario es una de las prioridades del proyecto.

---

## Limitaciones actuales

TzLang está en desarrollo temprano y es honesto sobre lo que todavía no hace.

**Unicode.** Los textos se tratan como bytes, no como caracteres. Esto se nota en cuanto aparecen tildes o eñes:

```tz
imprimir largo("año")
imprimir mayusculas("año")
```

```
4
AñO
```

`largo("año")` devuelve `4` en lugar de `3`, y `mayusculas` deja intacta la `ñ` porque ocupa dos bytes. Es la limitación más visible para un lenguaje pensado en español, y la primera de la lista para arreglar.

**Paso de argumentos.** Las funciones reciben **copias** de listas y diccionarios, no referencias. Modificar una dentro de una función no afecta a la de fuera; hay que devolverla. Es lo contrario de Python o JavaScript, y sorprende a quien viene de ahí.

**Límites de seguridad.** Una función puede anidar hasta 2.000 llamadas y una expresión 500 niveles. Pasados esos, TzLang corta con un error en lugar de agotar la pila del proceso. Son márgenes muy por encima de lo que necesita un programa normal.

**Rendimiento de los diccionarios.** Buscar una clave recorre el diccionario entero. Con unos cientos de claves no se nota; con miles, sí. Para colecciones grandes, una lista es mucho más rápida.

**Ausencias del lenguaje.** No hay módulos ni importaciones, clases, funciones anónimas, generadores, conjuntos, tuplas, recolector de basura ni enteros de precisión arbitraria.

---

## Roadmap

Lo que ya está terminado en la **0.4.0**: lexer, parser, AST, intérprete y runtime propios; variables y los siete tipos; operadores aritméticos —resto incluido—, de comparación y lógicos; sintaxis comparativa en español; condicionales con `sino si`, bucles, `romper` y `continuar`; secuencias de escape en textos; funciones con parámetros, retorno, recursión y scope léxico; listas y diccionarios anidados con copia profunda, índices negativos y concatenación; 17 funciones incorporadas, con entrada por teclado; CLI con códigos de salida diferenciados; notas de diagnóstico por categoría de error; cinco lecciones en `education/`; suite de 173 pruebas verde también bajo ASan y UBSan; CI que compila y prueba en Linux, macOS y Windows; instaladores nativos para macOS, Windows y Linux publicados automáticamente en cada versión.

Lo siguiente, por orden de prioridad:

- Soporte real de Unicode en textos y funciones de cadena. Es la limitación más visible para un lenguaje en español.
- Seguir ampliando `education/` con más lecciones y ejercicios.
- Mensajes de error con número de línea y contexto en todas las etapas.
- Decidir si el paso de argumentos debe seguir siendo por copia.
- Sistema de módulos.

No hay fechas comprometidas: es un proyecto en desarrollo.

---

## Documentación

La referencia completa del lenguaje, sección por sección, está en **[docs/language.md](docs/language.md)**.

---

## Licencia

TzLang se distribuye bajo la licencia **MIT**. Consulta el archivo [LICENSE](LICENSE) para el texto completo.
