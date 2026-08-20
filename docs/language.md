# TzLang 0.2.1 - Documentación del Lenguaje

## 1. Hola mundo

Un programa básico:

```
imprimir "Hola desde TzLang"
```

Se ejecuta mediante:

```bash
./build/tzc programa.tz
```

También existen opciones de ayuda y versión:

```bash
./build/tzc --help
./build/tzc -h
./build/tzc --version
./build/tzc -v
```

## 2. Variables

Las variables se declaran utilizando `variable`:

```
variable nombre = "Carlos"
variable edad = 20
variable activo = verdadero
```

Las variables pueden almacenar diferentes tipos de valores. También pueden modificarse posteriormente:

```
variable edad = 20
edad = 21
imprimir edad
```

## 3. Tipos de datos

TzLang actualmente soporta:

- `numero`
- `decimal`
- `texto`
- `booleano`
- `nulo`
- `lista`
- `diccionario`

### Numero

Los números enteros utilizan el tipo `numero`:

```
variable edad = 20
variable cantidad = 100
```

El rango corresponde al `int` utilizado por el runtime. Los valores fuera de rango producen un error.

### Decimal

Los números con parte decimal utilizan `decimal`:

```
variable precio = 19.99
variable promedio = 4.5
```

### Texto

Los textos se escriben entre comillas:

```
variable nombre = "Carlos"
imprimir nombre
```

### Booleanos

TzLang utiliza:

- `verdadero`
- `falso`

Ejemplo:

```
variable activo = verdadero
```

### Nulo

El valor vacío se representa mediante:

```
nulo
```

## 4. Operadores aritméticos

TzLang soporta: `+`, `-`, `*`, `/`

Ejemplo:

```
variable suma = 10 + 5
variable resta = 10 - 5
variable multiplicacion = 10 * 5
variable division = 10 / 5
```

La precedencia matemática se mantiene:

```
variable resultado = 2 + 3 * 4
imprimir resultado
```

El resultado es: `14`

Los paréntesis permiten modificar la precedencia:

```
variable resultado = (2 + 3) * 4
```

Resultado: `20`

## 5. Menos unario

TzLang permite utilizar `-` como operador unario:

```
variable x = -5
variable y = -3.14
```

También funciona dentro de expresiones:

```
imprimir -5 * 2
```

Resultado: `-10`

Los signos pueden encadenarse:

```
imprimir --5
```

Resultado: `5`

La resta binaria continúa funcionando normalmente:

```
imprimir 10 - -3
```

Resultado: `13`

## 6. Comparaciones

TzLang permite comparar valores mediante: `==`, `!=`, `<`, `>`, `<=`, `>=`

Ejemplo:

```
si edad >= 18
    imprimir "Mayor de edad"
fin
```

La igualdad también funciona con estructuras:

```
variable a = [1, 2, 3]
variable b = [1, 2, 3]
imprimir a == b
```

Los diccionarios se comparan por sus pares y no por el orden de inserción:

```
{"a": 1, "b": 2} == {"b": 2, "a": 1}
```

## 7. Operadores lógicos

TzLang utiliza: `y`, `o`, `no`

Ejemplo:

```
si edad >= 18 y activo
    imprimir "Puede entrar"
fin
```

También:

```
si edad < 18 o activo == falso
    imprimir "No puede entrar"
fin
```

Negación:

```
si no activo
    imprimir "Inactivo"
fin
```

## 8. Condicionales

La estructura básica es:

```
si condicion
    instrucciones
fin
```

Ejemplo:

```
variable edad = 20
si edad >= 18
    imprimir "Mayor de edad"
fin
```

También existe `sino`:

```
si edad >= 18
    imprimir "Mayor de edad"
sino
    imprimir "Menor de edad"
fin
```

Los bloques pueden anidarse:

```
si edad >= 18
    si activo
        imprimir "Activo y mayor de edad"
    fin
fin
```

## 9. Mientras

Los bucles `mientras` ejecutan un bloque mientras una condición sea verdadera:

```
variable contador = 0
mientras contador < 5
    imprimir contador
    contador = contador + 1
fin
```

Resultado:

```
0
1
2
3
4
```

Los bucles pueden estar anidados.

## 10. Para cada

`para cada` permite recorrer listas:

```
variable numeros = [1, 2, 3]
para cada numero en numeros
    imprimir numero
fin
```

También permite recorrer diccionarios. Cuando se recorre un diccionario, la variable recibe sus claves, en orden de inserción:

```
variable persona = {
    "nombre": "Carlos",
    "edad": 20
}
para cada clave en persona
    imprimir clave
fin
```

Resultado:

```
nombre
edad
```

## 11. Romper y continuar

`romper` termina el bucle actual:

```
variable i = 0
mientras i < 10
    si i == 5
        romper
    fin
    imprimir i
    i = i + 1
fin
```

`continuar` salta a la siguiente iteración:

```
para cada numero en [1, 2, 3, 4]
    si numero == 2
        continuar
    fin
    imprimir numero
fin
```

Estas instrucciones solamente pueden utilizarse dentro de bucles.

## 12. Funciones

Las funciones se declaran mediante `funcion`:

```
funcion saludar()
    imprimir "Hola"
fin
```

Se llaman utilizando paréntesis:

```
saludar()
```

## 13. Parámetros

Las funciones pueden recibir parámetros:

```
funcion saludar(nombre)
    imprimir nombre
fin
saludar("Carlos")
```

También pueden recibir múltiples parámetros:

```
funcion sumar(a, b)
    retornar a + b
fin
imprimir sumar(10, 20)
```

## 14. Retorno

Las funciones pueden devolver valores mediante `retornar`:

```
funcion sumar(a, b)
    retornar a + b
fin
variable resultado = sumar(10, 20)
imprimir resultado
```

Resultado: `30`

Las funciones pueden retornar listas, diccionarios y otros valores.

## 15. Recursión

Las funciones pueden llamarse a sí mismas.

Ejemplo factorial:

```
funcion factorial(n)
    si n <= 1
        retornar 1
    fin
    retornar n * factorial(n - 1)
fin
imprimir factorial(5)
```

Resultado: `120`

## 16. Scope

Las funciones tienen su propio ámbito:

```
variable mensaje = "global"
funcion prueba()
    variable mensaje = "local"
    imprimir mensaje
fin
prueba()
imprimir mensaje
```

Resultado:

```
local
global
```

Los parámetros de las funciones reciben copias independientes de los valores. Por ejemplo:

```
funcion agregar_valor(lista)
    agregar(lista, 99)
fin
variable numeros = [1, 2]
agregar_valor(numeros)
imprimir numeros
```

El parámetro es una copia profunda, por lo que el `numeros` original permanece:

```
[1, 2]
```

## 17. Listas

Las listas se crean mediante corchetes:

```
variable numeros = [1, 2, 3]
```

Pueden contener diferentes tipos:

```
variable datos = [
    "Carlos",
    20,
    verdadero
]
```

También pueden estar anidadas:

```
variable matriz = [
    [1, 2],
    [3, 4]
]
```

## 18. Indexación de listas

Los elementos se acceden mediante índices:

```
variable numeros = [10, 20, 30]
imprimir numeros[0]
imprimir numeros[1]
```

Los índices comienzan en 0. También pueden utilizarse expresiones:

```
variable i = 1
imprimir numeros[i]
```

Un índice fuera de rango produce un error.

## 19. Modificación de listas

Los elementos pueden modificarse:

```
variable numeros = [1, 2, 3]
numeros[1] = 20
imprimir numeros
```

Resultado: `[1, 20, 3]`

La modificación puede utilizar estructuras anidadas:

```
variable datos = {
    "numeros": [1, 2, 3]
}
datos["numeros"][1] = 99
```

## 20. Diccionarios

Los diccionarios contienen pares clave-valor:

```
variable persona = {
    "nombre": "Carlos",
    "edad": 20,
    "activo": verdadero
}
```

Las claves actualmente deben ser textos.

## 21. Acceso a diccionarios

Los valores se obtienen mediante indexación:

```
imprimir persona["nombre"]
imprimir persona["edad"]
```

El acceso puede estar anidado:

```
variable usuario = {
    "datos": {
        "edad": 20
    }
}
imprimir usuario["datos"]["edad"]
```

También pueden mezclarse listas y diccionarios:

```
variable usuarios = [
    {
        "nombre": "Ana",
        "edad": 20
    },
    {
        "nombre": "Carlos",
        "edad": 25
    }
]
imprimir usuarios[0]["nombre"]
imprimir usuarios[1]["edad"]
```

## 22. Modificación de diccionarios

Una clave existente puede actualizarse:

```
persona["edad"] = 21
```

También puede crearse una nueva clave:

```
persona["pais"] = "Colombia"
```

La nueva clave se agrega al final del orden de inserción. No se crean automáticamente niveles intermedios:

```
datos["inexistente"]["valor"] = 1
```

produce un error si `inexistente` no existe.

## 23. Orden de los diccionarios

Los diccionarios conservan el orden de inserción:

```
variable persona = {
    "nombre": "Carlos",
    "edad": 20,
    "pais": "Colombia"
}
```

El recorrido produce: `nombre`, `edad`, `pais`

La implementación utiliza un array dinámico y búsqueda lineal.

## 24. Funciones incorporadas

TzLang incluye funciones incorporadas para trabajar con valores.

### largo

Obtiene el tamaño de un texto, lista o diccionario:

```
imprimir largo("Hola")
imprimir largo([1, 2, 3])
imprimir largo({"a": 1, "b": 2})
```

Actualmente, `largo()` sobre textos cuenta bytes UTF-8, no caracteres Unicode.

### tipo

Obtiene el tipo de un valor:

```
imprimir tipo(10)
imprimir tipo(3.14)
imprimir tipo("Hola")
imprimir tipo([1, 2])
```

### texto

Convierte valores compatibles a texto:

```
imprimir texto(10)
imprimir texto(3.14)
imprimir texto(verdadero)
```

Actualmente no convierte listas ni diccionarios.

### numero

Convierte un valor a `numero`:

```
imprimir numero("20")
imprimir numero(20.5)
imprimir numero(verdadero)
```

Las cadenas deben representar un entero completo. Por ejemplo:

```
numero("12")
```

es válido. Pero:

```
numero("12abc")
numero("1.5")
```

produce un error.

### decimal

Convierte valores a `decimal`:

```
imprimir decimal("3.14")
imprimir decimal(10)
imprimir decimal(verdadero)
```

### agregar

Agrega un elemento a una lista:

```
variable numeros = [1, 2, 3]
agregar(numeros, 4)
imprimir numeros
```

Resultado: `[1, 2, 3, 4]`

`agregar()` modifica la lista recibida y devuelve `nulo`. También funciona con estructuras anidadas:

```
agregar(datos["numeros"], 4)
```

### eliminar

Elimina un elemento de una lista:

```
variable numeros = [10, 20, 30]
variable eliminado = eliminar(numeros, 1)
imprimir eliminado
```

Resultado: `20`

También puede eliminar una clave de un diccionario:

```
variable persona = {
    "nombre": "Carlos",
    "edad": 20
}
variable edad = eliminar(persona, "edad")
imprimir edad
```

`eliminar()` devuelve el valor eliminado.

### contiene

Comprueba si una lista contiene un valor:

```
variable numeros = [1, 2, 3]
imprimir contiene(numeros, 2)
```

Resultado: `verdadero`

También funciona con diccionarios:

```
variable persona = {
    "nombre": "Carlos"
}
imprimir contiene(persona, "nombre")
```

### unir

Une una lista de textos utilizando un separador:

```
variable partes = ["Hola", "mundo"]
imprimir unir(partes, " ")
```

Resultado: `Hola mundo`

Todos los elementos de la lista deben ser textos.

### separar

Divide un texto utilizando un separador:

```
variable partes = separar("uno,dos,tres", ",")
imprimir partes
```

Resultado: `["uno", "dos", "tres"]`

El separador no puede ser vacío.

### mayusculas

Convierte texto ASCII a mayúsculas:

```
imprimir mayusculas("hola")
```

Resultado: `HOLA`

### minusculas

Convierte texto ASCII a minúsculas:

```
imprimir minusculas("HOLA")
```

Resultado: `hola`

Las operaciones de texto actualmente no implementan transformación Unicode completa.

### absoluto

Obtiene el valor absoluto:

```
imprimir absoluto(-5)
imprimir absoluto(-3.14)
```

Conserva el tipo del valor.

### redondear

Redondea un decimal y devuelve un `numero`:

```
imprimir redondear(3.5)
```

Resultado: `4`

El redondeo de valores exactamente intermedios se realiza alejándose de cero: `2.5 → 3`, `-2.5 → -3`

### claves

Obtiene las claves de un diccionario como una lista:

```
variable persona = {
    "nombre": "Carlos",
    "edad": 20
}
imprimir claves(persona)
```

Resultado: `["nombre", "edad"]`

El orden es el orden de inserción.

### valores

Obtiene los valores de un diccionario como una lista:

```
variable persona = {
    "nombre": "Carlos",
    "edad": 20
}
imprimir valores(persona)
```

Los valores se devuelven como copias profundas.

## 25. Literales multilínea

Las listas, diccionarios y llamadas pueden escribirse en varias líneas.

Ejemplo:

```
variable numeros = [
    1,
    2,
    3
]
```

Diccionarios:

```
variable persona = {
    "nombre": "Carlos",
    "edad": 20,
    "activo": verdadero
}
```

También pueden anidarse:

```
variable usuario = {
    "datos": {
        "nombre": "Carlos",
        "edad": 20
    },
    "roles": [
        "admin",
        "usuario"
    ]
}
```

Los saltos de línea fuera de estos delimitadores continúan funcionando como terminadores de instrucciones.

## 26. Copia profunda

TzLang utiliza copia profunda para sus valores estructurados:

```
variable a = {
    "datos": {
        "edad": 20
    }
}
variable b = a
b["datos"]["edad"] = 99
imprimir a["datos"]["edad"]
```

El resultado continúa siendo: `20`

Lo mismo aplica a listas anidadas. Esto evita que dos variables compartan accidentalmente la misma estructura mutable.

## 27. Valores verdaderos y falsos

Los valores estructurados y otros tipos participan en las condiciones. Por ejemplo:

```
si []
    imprimir "verdadero"
fin
```

Una lista vacía es falsa. Un diccionario vacío también es falso:

```
si {}
    imprimir "verdadero"
fin
```

Los valores vacíos y `nulo` tienen comportamiento falso dentro de condiciones.

## 28. Errores

TzLang diferencia errores de diferentes etapas. El CLI utiliza estos códigos:

| Código | Significado |
|--------|-------------|
| 0 | ejecución correcta |
| 1 | error de argumentos/opciones |
| 2 | error de archivo o extensión |
| 3 | error de lexer, parser o interpreter |

Ejemplo:

```bash
./build/tzc
```

produce un error de argumentos.

Un archivo inexistente:

```bash
./build/tzc inexistente.tz
```

produce un error de archivo.

Un programa con sintaxis inválida produce un error de parser.

## 29. Archivos .tz

Los programas de TzLang utilizan la extensión: `.tz`

Ejemplo: `programa.tz`

El CLI rechaza archivos que no tengan esta extensión. Los archivos pueden contener:

- UTF-8
- BOM UTF-8
- CRLF
- LF
- archivo vacío
- archivo sin salto de línea final

El BOM UTF-8 se descarta automáticamente. Los caracteres Unicode todavía no forman parte de la sintaxis completa del lenguaje.

## 30. CLI

La forma básica de ejecutar un programa es:

```bash
tzc programa.tz
```

Ayuda:

```bash
tzc --help
tzc -h
```

Versión:

```bash
tzc --version
tzc -v
```

Actualmente la versión definida en el proyecto es: **TzLang 0.2.1**

Una opción desconocida produce un error:

```bash
tzc --banana
```

## 31. Compilación desde código fuente

TzLang utiliza un Makefile.

**Compilar:**

```bash
make
```

**Ejecutar pruebas:**

```bash
make test
```

**Compilación de depuración:**

```bash
make debug
```

El binario de depuración se genera como: `build/tzc-debug`

**Compilación con AddressSanitizer y UndefinedBehaviorSanitizer:**

```bash
make asan
```

Genera: `build/tzc-asan`

**Limpiar los artefactos:**

```bash
make clean
```

## 32. Estructura del proyecto

La estructura principal es:

```
TzLang/
├── src/
│   ├── ast/
│   │   ├── ast.c
│   │   └── ast.h
│   ├── interpreter/
│   │   ├── interpreter.c
│   │   └── interpreter.h
│   ├── io/
│   │   ├── file.c
│   │   └── file.h
│   ├── lexer/
│   │   ├── lexer.c
│   │   └── lexer.h
│   ├── parser/
│   │   ├── parser.c
│   │   └── parser.h
│   ├── runtime/
│   │   ├── operations.c
│   │   ├── operations.h
│   │   ├── value.c
│   │   └── value.h
│   ├── main.c
│   └── version.h
├── examples/
├── tests/
├── docs/
├── Makefile
└── .gitignore
```

## 33. Arquitectura

El código fuente pasa por varias etapas:

### Lexer

Convierte caracteres en tokens:

```
Código fuente → Lexer → Tokens
```

### Parser

Convierte los tokens en un AST:

```
Tokens → Parser → AST
```

### AST

Representa estructuralmente el programa. Entre los nodos utilizados se encuentran conceptos como:

- literal
- variable
- asignación
- operación
- condición
- bucle
- función
- llamada
- lista
- diccionario
- indexación
- retorno

### Interpreter

Evalúa el AST y ejecuta las instrucciones.

### Runtime

Gestiona los valores y operaciones:

- numero
- decimal
- texto
- booleano
- nulo
- lista
- diccionario

Las estructuras compuestas utilizan copia profunda y liberación recursiva.

## 34. Seguridad de memoria

El proyecto se prueba con:

```bash
make asan
```

La suite actual contiene pruebas de:

- errores de tipos
- índices inválidos
- claves inexistentes
- errores de parser
- errores léxicos
- overflow entero
- división por cero
- estructuras anidadas
- copia profunda
- funciones
- recursión
- CLI
- archivos
- built-ins

La suite actual contiene **138 pruebas**, todas deben finalizar correctamente.

Resultado esperado:

```
Tests:  138
Passed: 138
Failed: 0
All tests passed.
```

## 35. Limitaciones actuales

TzLang todavía tiene algunas características fuera del alcance actual:

### Unicode completo

Las operaciones de texto actualmente trabajan principalmente con ASCII. Por ejemplo, `largo()` cuenta bytes y no caracteres Unicode.

### Módulos

Todavía no existe un sistema de `importar ...`

### Clases

No existe programación orientada a objetos.

### Lambdas

No existe sintaxis para funciones anónimas.

### Generadores

No existe soporte para generadores.

### Sets

No existe un tipo `set`.

### Tuplas

No existe un tipo `tupla`.

### Garbage Collector

La gestión de memoria se realiza mediante ownership y liberación explícita del runtime. No existe un garbage collector.

### Enteros grandes

`numero` utiliza un entero de 32 bits. Los desbordamientos producen errores en lugar de promover automáticamente el valor.

## 36. Ejemplo completo

El siguiente programa combina variables, funciones, listas, diccionarios, condiciones y bucles:

```
funcion mostrar_usuario(usuario)
    imprimir "Usuario: " + usuario["nombre"]
    imprimir "Edad: " + texto(usuario["edad"])
    si usuario["activo"]
        imprimir "Estado: activo"
    sino
        imprimir "Estado: inactivo"
    fin
fin

variable usuarios = [
    {
        "nombre": "Ana",
        "edad": 20,
        "activo": verdadero
    },
    {
        "nombre": "Carlos",
        "edad": 25,
        "activo": falso
    }
]

para cada usuario en usuarios
    mostrar_usuario(usuario)
fin
```

## 37. Estado de TzLang

La versión actual es: **TzLang 0.2.1**

El lenguaje actualmente dispone de:

- lexer
- parser
- AST
- interpreter
- runtime
- variables
- tipos básicos
- operadores
- control de flujo
- funciones
- recursión
- listas
- diccionarios
- indexación anidada
- funciones incorporadas
- CLI
- lectura de archivos `.tz`
- sistema de tests
- Makefile
- builds de debug
- builds con sanitizers

El objetivo de las siguientes versiones es ampliar las capacidades del lenguaje sin romper la compatibilidad existente. La siguiente etapa puede ser **Unicode**, seguida de **módulos**, marcando el inicio de **TzLang 0.2**.
