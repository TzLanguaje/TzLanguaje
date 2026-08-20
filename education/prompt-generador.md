# Prompt para generar ejercicios de TzLang

Copia todo lo que hay debajo de la línea
y pégalo en Claude. Está pensado para
que genere ejercicios de nivel junior
que **ejecuten de verdad**.

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
```

**Tipos:** `numero` (entero), `decimal`, `texto`, `booleano` (`verdadero` / `falso`), `nulo`, `lista`, `diccionario`.

**Comparaciones** — se escriben con palabras, no con símbolos:

```
es mayor que            es menor que
es mayor o igual que    es menor o igual que
es igual a              es diferente de
```

**Lógicos:** `y`, `o`, `no`

**Aritméticos:** `+`, `-`, `*`, `/`

**Condicional:**

```tz
si edad es mayor o igual que 18
    imprimir "Mayor de edad"
sino
    imprimir "Menor de edad"
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

variable persona = {"nombre": "Ana", "edad": 20}
imprimir persona["nombre"]
```

**Las 16 funciones incorporadas, y no hay más:**

`largo(x)` · `tipo(x)` · `texto(x)` · `numero(x)` · `decimal(x)` · `agregar(lista, v)` · `eliminar(x, k)` · `contiene(x, v)` · `unir(lista, sep)` · `separar(txt, sep)` · `mayusculas(txt)` · `minusculas(txt)` · `absoluto(x)` · `redondear(x)` · `claves(dic)` · `valores(dic)`

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

## `agregar` y `eliminar` devuelven `nulo`

```tz
lista = agregar(lista, 3)   // MAL: lista pasa a valer nulo
agregar(lista, 3)           // bien: se llama y ya esta
```

Es el hábito de `lista.append()` de Python. Aquí destruye la lista.

## Lo que NO existe — no lo uses

- **`sino si`** no existe. Para encadenar condiciones, anida un `si` completo dentro del `sino`, y **cada nivel suma un `fin` al final**:

  ```tz
  si x es mayor que 10
      imprimir "grande"
  sino
      si x es mayor que 3
          imprimir "mediano"
      sino
          imprimir "pequeno"
      fin
  fin
  ```

  Con tres o más ramas es muy fácil perder la cuenta de los `fin`. Cuéntalos.
- **`+=`, `-=`, `++`, `--`** no existen. Escribe `x = x + 1`.
- **`para i = 0 hasta 10`** no existe. Solo existe `para cada x en lista`. Para contar, usa `mientras`.
- **`>`, `<`, `>=`, `<=`, `==`, `!=`** no existen como símbolos. Usa las formas con palabras.
- No hay `entrada`, `leer` ni forma de pedir datos al usuario. Los valores van escritos en el programa.
- No hay clases, objetos, módulos, `importar`, ni manejo de excepciones.
- No hay operador de módulo `%`. Si necesitas el resto, calcúlalo: `a - (a / b) * b`. Funciona porque la división entre enteros trunca.
- **Cuidado con la división entera:** entre dos enteros, `10 / 3` da `3`, no `3.33`.
- **Y cuidado con el orden al convertir.** `decimal()` hay que aplicarlo **antes** de dividir, no después:

  ```tz
  decimal(10 / 3)          // 3        <- MAL: la division ya trunco
  decimal(10) / 3          // 3.33333  <- bien
  decimal(10) / decimal(3) // 3.33333  <- bien
  10 / decimal(3)          // 3.33333  <- bien
  ```

  Basta con que **uno** de los dos sea decimal. `decimal(a / b)` es el error clásico: convierte un resultado que ya perdió los decimales.
- **`%`** no existe como operador. `7 % 3` da error del lexer.
- **`x++` y `x--`** no existen.
- **Comillas simples** no existen: los textos van con `"` dobles. `'Hola'` da error.
- **Notación de punto** no existe: es `persona["nombre"]`, nunca `persona.nombre`.
- **No hay secuencias de escape.** `\n` sale literal como barra y ene, y `\"` da error del lexer. Para varias líneas, usa varios `imprimir`. No se pueden meter comillas dobles dentro de un texto.
- **`texto()` no convierte listas ni diccionarios.** Para verlos, `imprimir lista` directamente.
- `romper` y `continuar` solo valen dentro de un `mientras` o un `para cada`.
- Los bloques se cierran **siempre** con `fin`.
- Cada instrucción va en su propia línea.

## Detalles de comportamiento

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
