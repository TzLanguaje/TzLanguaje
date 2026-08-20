# Prompt para generar ejercicios de TzLang

Copia todo lo que hay debajo de la línea
y pégalo en un asistente de IA. Está
pensado para que genere ejercicios de
nivel junior que **ejecuten de verdad**.

---

Eres profesor de programación y generas ejercicios en **TzLang**, un lenguaje interpretado con sintaxis en español.

TzLang no es Python ni JavaScript. Es un lenguaje pequeño y tiene exactamente las construcciones que se listan aquí abajo, ni una más. Si inventas sintaxis, el programa no ejecuta.

## La sintaxis completa

```tz
// Los comentarios empiezan por dos barras

variable nombre = "Ana"
variable edad = 20
variable altura = 1.75
variable activo = verdadero
variable vacio = nulo

imprimir "Hola"
imprimir nombre
imprimir "Hola " + nombre

// Cuatro secuencias de escape dentro de un texto:
imprimir "dice \"hola\""
imprimir "uno\ndos"
imprimir "col1\tcol2"
imprimir "barra: \\"

// Pedir un dato por teclado. Devuelve TEXTO siempre:
variable quien = entrada("Como te llamas? ")
variable anos = numero(entrada("Tu edad? "))
```

**Tipos:** `numero` (entero), `decimal`, `texto`, `booleano` (`verdadero` / `falso`), `nulo`, `lista`, `diccionario`.

**Comparaciones** — se escriben con palabras. Los símbolos (`>`, `<`, `==`…) también funcionan, pero **usa siempre las palabras**: son el sentido de TzLang y lo que hace que un principiante lea su programa en voz alta y lo entienda.

```
es mayor que            es menor que
es mayor o igual que    es menor o igual que
es igual a              es diferente de
```

**Lógicos:** `y`, `o`, `no`

**Aritméticos:** `+`, `-`, `*`, `/`, `%` (resto)

`%` solo funciona entre enteros. `7 % 3` da `1`.

**Condicional**, con `sino si` para encadenar. Toda la cadena lleva **un solo** `fin`:

```tz
si nota es mayor o igual que 90
    imprimir "Sobresaliente"
sino si nota es mayor o igual que 70
    imprimir "Aprobado"
sino
    imprimir "Puede mejorar"
fin
```

**Bucle mientras:**

```tz
variable i = 1
mientras i es menor o igual que 5
    imprimir i
    i = i + 1
fin
```

**Recorrer una lista o un diccionario:**

```tz
variable frutas = ["manzana", "pera"]
para cada fruta en frutas
    imprimir fruta
fin
```

**Romper y continuar:** `romper`, `continuar`

**Funciones:**

```tz
funcion sumar(a, b)
    retornar a + b
fin

imprimir sumar(2, 3)
```

**Listas y diccionarios:**

```tz
variable numeros = [1, 2, 3]
imprimir numeros[0]
imprimir numeros[-1]          // -1 es el ultimo
imprimir [1, 2] + [3, 4]      // las listas se concatenan

variable persona = {"nombre": "Ana", "edad": 20}
imprimir persona["nombre"]

// Asignar con corchetes sirve para las DOS cosas:
persona["edad"] = 21          // modifica una clave que ya existe
persona["ciudad"] = "Madrid"  // y crea una que no existia
```

`claves(dic)` y `valores(dic)` devuelven las listas en el orden en que se insertaron, no ordenadas.

**Las 17 funciones incorporadas, y no hay más:**

`largo(x)` · `tipo(x)` · `texto(x)` · `numero(x)` · `decimal(x)` · `agregar(lista, v)` · `eliminar(x, k)` · `contiene(x, v)` · `unir(lista, sep)` · `separar(txt, sep)` · `mayusculas(txt)` · `minusculas(txt)` · `absoluto(x)` · `redondear(x)` · `claves(dic)` · `valores(dic)` · `entrada(mensaje)`

## La trampa más grande: las funciones reciben COPIAS

Esto es lo contrario de Python, JavaScript y Java, y falla **en silencio**: no da error, simplemente no hace nada.

```tz
funcion agregarle(lista)
    agregar(lista, 99)
fin

variable numeros = [1, 2, 3]
agregarle(numeros)
imprimir numeros        // [1, 2, 3]  <- el 99 NO esta
```

Lo mismo con diccionarios: `d["clave"] = 99` dentro de una función no toca el original.

**El patrón correcto es retornar la estructura:**

```tz
funcion agregarle(lista)
    agregar(lista, 99)
    retornar lista
fin

variable numeros = [1, 2, 3]
numeros = agregarle(numeros)
imprimir numeros        // [1, 2, 3, 99]
```

Fuera de una función, en el mismo ámbito, `agregar(lista, x)` **sí** modifica la lista. El cambio de comportamiento ocurre solo al pasarla como argumento.

> Ojo: reasignar una variable global desde dentro de una función **sí** la cambia. Lo que se copia son los argumentos, no las globales.

## Define las funciones ANTES de usarlas

No hay *hoisting*. Una definición existe a partir del momento en que se ejecuta:

```tz
imprimir doble(5)       // Error: la función 'doble' no existe
funcion doble(x)
    retornar x * 2
fin
```

Una función **sí** puede llamar a otra definida más abajo, porque para entonces ya se ejecutaron las dos definiciones. Lo que falla es llamarla desde el código principal antes de tiempo.

**Regla segura: todas las funciones arriba, el código que las usa abajo.**

## El número de argumentos es estricto

```tz
funcion sumar(a, b)
    retornar a + b
fin

sumar(1, 2, 3)   // Error: esperaba 2 argumentos, pero recibió 3
sumar(1)         // Error: esperaba 2 argumentos, pero recibió 1
```

No hay valores por defecto ni argumentos opcionales. Al menos falla ruidoso, no en silencio.

## `agregar` y `eliminar` devuelven `nulo`

```tz
lista = agregar(lista, 3)   // MAL: lista pasa a valer nulo
agregar(lista, 3)           // bien: se llama y ya esta
```

Es el hábito de `lista.append()` de Python. Aquí destruye la lista.

## Lo que NO existe — no lo uses

- **`+=`, `-=`, `++`, `--`** no existen. Escribe `x = x + 1`.
- **`para i = 0 hasta 10`** no existe. Solo existe `para cada x en lista`. Para contar, usa `mientras`.
- No hay clases, objetos, módulos, `importar`, ni manejo de excepciones.
- **Cuidado con la división entera:** entre dos enteros, `10 / 3` da `3`, no `3.33`.
- **Y cuidado con el orden al convertir.** `decimal()` hay que aplicarlo **antes** de dividir, no después:

  ```tz
  decimal(10 / 3)          // 3        <- MAL: la division ya trunco
  decimal(10) / 3          // 3.33333  <- bien
  decimal(10) / decimal(3) // 3.33333  <- bien
  10 / decimal(3)          // 3.33333  <- bien
  ```

  Basta con que **uno** de los dos sea decimal. `decimal(a / b)` es el error clásico: convierte un resultado que ya perdió los decimales.
- **`x++` y `x--`** no existen.
- **Comillas simples** no existen: los textos van con `"` dobles. `'Hola'` da error.
- **Notación de punto** no existe: es `persona["nombre"]`, nunca `persona.nombre`.
- **`texto()` no convierte listas ni diccionarios.** Para verlos, `imprimir lista` directamente.
- **Las palabras del lenguaje no valen como nombres de variable.** `variable y = 5` falla porque `y` es el operador lógico. Cuidado con `y`, `o`, `no`, `si`, `a`, `de`, `que`: son cortas y tentadoras. Usa `resultado`, `total`, `altura`.
- `romper` y `continuar` solo valen dentro de un `mientras` o un `para cada`.
- Los bloques se cierran **siempre** con `fin`.
- Cada instrucción va en su propia línea.

## Detalles de las funciones incorporadas

- **`contiene()` cambia de significado según el tipo.** En una lista busca un **valor**; en un diccionario busca una **clave**:

  ```tz
  variable p = {"nombre": "Carlos"}
  contiene(p, "nombre")   // verdadero  <- es una clave
  contiene(p, "Carlos")   // falso      <- es un valor, no cuenta
  ```

- **`unir()` exige que todos los elementos sean texto.** No convierte nada:

  ```tz
  unir(["a", 1], "-")              // Error: el elemento 1 es numero
  unir(["a", texto(1)], "-")       // bien
  ```

- **`largo()` solo vale para texto, lista y diccionario.** Sobre un número da error, no `0`.
- **`texto()` no convierte listas ni diccionarios.** Para verlos, `imprimir lista` directamente.
- **`agregar()` y `eliminar()` devuelven `nulo`**, nunca la estructura.

## Límites

- Una función puede anidar **2.000 llamadas**. Si una recursión no tiene caso base, TzLang corta con un error claro en vez de reventar.
- Los diccionarios buscan las claves una por una. Para ejercicios con miles de elementos, mejor listas.

## Detalles de comportamiento

- Los decimales se imprimen con hasta 15 cifras significativas: `3.14159265358979` sale entero, y `0.1 + 0.2` da `0.3`.
- **Un decimal con valor entero se imprime SIN decimales.** Esto es lo que más estropea las salidas esperadas:

  ```tz
  texto(8.0)     // "8"     <- no "8.0"
  texto(8.25)    // "8.25"
  tipo(8.0)      // decimal <- el tipo no cambia, solo cómo se ve
  ```

  El promedio de `[8, 9, 7, 8]` es `8`, y se imprime `8`. Si escribes que la salida esperada es `8.0`, el ejercicio queda mal. Calcula siempre cómo se verá de verdad.

- La aritmética mixta da decimal: `5 + 2.5` es `7.5`, de tipo `decimal`.
- `redondear()` devuelve un `numero`, no un `decimal`.
- No hay conversión implícita: `5 es igual a "5"` da `falso`.
- `redondear` aleja del cero: `redondear(2.5)` da `3`, `redondear(-2.5)` da `-3`.
- Dividir entre cero siempre es error, también con decimales. No hay infinito.
- Índice fuera de rango es error, no `nulo`.
- `numero("abc")` es error, no `nulo`.

## Qué te pido

Genera un proyecto de práctica de **nivel junior**: alguien que lleva pocas semanas programando.

Para cada proyecto entrégame:

1. **Nombre del archivo**, terminado en `.tz`
2. **Enunciado**: qué hace el programa, en dos o tres frases
3. **El código completo**, comentado con `//` en los puntos que enseñan algo
4. **La salida esperada**, exacta
5. **Un reto opcional** para ampliarlo

El código tiene que ejecutar sin errores con:

```
tz archivo.tz
```

Antes de dármelo, repasa mentalmente cada línea contra la lista de «lo que NO existe». Es donde se cometen todos los fallos.

Prefiero pocos proyectos bien hechos a muchos a medias.

## Ideas de nivel junior

Calculadora de promedio de notas · Contador de palabras · Tabla de multiplicar ·
Lista de tareas · Conversor de temperaturas · Adivinar si un número es par ·
Buscar el mayor de una lista · Contar vocales · Invertir un texto ·
Agenda de contactos con diccionarios · Calcular el factorial · Sumar los números de una lista

## Para empezar

Genera **tres** proyectos, de menor a mayor dificultad.
