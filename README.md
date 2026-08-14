# TzLang

<p align="center">
  <strong>Un lenguaje de programación en español, construido desde cero en C.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/version-0.1.0-blue" alt="Version">
  <img src="https://img.shields.io/badge/language-C11-blue" alt="C11">
  <img src="https://img.shields.io/badge/tests-138%20passed-success" alt="Tests">
  <img src="https://img.shields.io/badge/status-development-orange" alt="Status">
</p>

---

## ¿Qué es TzLang?

**TzLang** es un lenguaje de programación interpretado con sintaxis en español, desarrollado desde cero utilizando **C11**.

El proyecto implementa su propio:

- Lexer
- Parser
- AST
- Interpreter
- Runtime
- Sistema de valores
- Sistema de funciones
- CLI
- Sistema de pruebas

Los programas escritos en TzLang utilizan la extensión:

```text
.tz
```

Ejemplo:

```
variable nombre = "Carlos"
variable edad = 20
si edad >= 18
    imprimir "Hola " + nombre
fin
```

---

## Características

### Lenguaje

- Variables
- `numero`
- `decimal`
- `texto`
- `booleano`
- `nulo`
- Listas
- Diccionarios
- Operadores aritméticos
- Operadores de comparación
- Operadores lógicos
- Menos unario
- Condicionales `si / sino`
- Bucles `mientras`
- Bucles `para cada`
- `romper`
- `continuar`
- Funciones
- Parámetros
- Retorno
- Recursión
- Scope léxico
- Indexación anidada
- Literales multilínea
- Copia profunda

### Built-ins

Actualmente TzLang incluye:

```
largo()
tipo()
texto()
numero()
decimal()
agregar()
eliminar()
contiene()
unir()
separar()
mayusculas()
minusculas()
absoluto()
redondear()
claves()
valores()
```

---

## Ejemplo

Un programa utilizando varias características del lenguaje:

```
funcion mostrar_usuario(usuario)
    imprimir "Nombre: " + usuario["nombre"]
    imprimir "Edad: " + texto(usuario["edad"])
    si usuario["activo"]
        imprimir "Activo"
    sino
        imprimir "Inactivo"
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

---

## Diccionarios

Los diccionarios utilizan claves de texto:

```
variable persona = {
    "nombre": "Carlos",
    "edad": 20,
    "pais": "Colombia"
}
imprimir persona["nombre"]
persona["edad"] = 21
persona["profesion"] = "Developer"
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
        "developer"
    ]
}
imprimir usuario["datos"]["edad"]
```

---

## Funciones

```
funcion sumar(a, b)
    retornar a + b
fin

variable resultado = sumar(10, 20)
imprimir resultado
```

Resultado:

```
30
```

También soporta recursión:

```
funcion factorial(n)
    si n <= 1
        retornar 1
    fin
    retornar n * factorial(n - 1)
fin

imprimir factorial(5)
```

Resultado:

```
120
```

---

## Arquitectura

TzLang procesa un programa mediante varias etapas:

```
             TzLang Source
                  │
                  ▼
               Lexer
                  │
                  ▼
                Tokens
                  │
                  ▼
                Parser
                  │
                  ▼
                 AST
                  │
                  ▼
             Interpreter
                  │
                  ▼
               Runtime
                  │
                  ▼
               Output
```

### Lexer

Convierte el código fuente en tokens.

### Parser

Analiza los tokens y construye el AST.

### AST

Representa la estructura del programa.

### Interpreter

Evalúa el AST y ejecuta el programa.

### Runtime

Gestiona valores, operaciones, listas, diccionarios y memoria.

---

## Estructura del proyecto

```
TzLang/
│
├── src/
│   ├── ast/
│   │   ├── ast.c
│   │   └── ast.h
│   │
│   ├── interpreter/
│   │   ├── interpreter.c
│   │   └── interpreter.h
│   │
│   ├── io/
│   │   ├── file.c
│   │   └── file.h
│   │
│   ├── lexer/
│   │   ├── lexer.c
│   │   └── lexer.h
│   │
│   ├── parser/
│   │   ├── parser.c
│   │   └── parser.h
│   │
│   ├── runtime/
│   │   ├── operations.c
│   │   ├── operations.h
│   │   ├── value.c
│   │   └── value.h
│   │
│   ├── main.c
│   └── version.h
│
├── examples/
├── tests/
├── docs/
├── Makefile
├── .gitignore
└── README.md
```

---

## Requisitos

Actualmente el proyecto utiliza:

- C11
- Clang o GCC
- Make
- POSIX shell para ejecutar la suite de tests

En macOS:

```bash
xcode-select --install
```

---

## Compilar

Clona el repositorio:

```bash
git clone https://github.com/tzerk-last/TzLanguaje.git
cd TzLanguaje
```

Compila:

```bash
make
```

El ejecutable se genera en:

```
build/tzc
```

---

## Ejecutar un programa

```bash
./build/tzc examples/hola.tz
```

Salida:

```
Hola desde TzLang
```

Otro ejemplo:

```bash
./build/tzc examples/diccionarios.tz
```

---

## CLI

Mostrar ayuda:

```bash
./build/tzc --help
```

o:

```bash
./build/tzc -h
```

Mostrar versión:

```bash
./build/tzc --version
```

o:

```bash
./build/tzc -v
```

Versión actual:

```
TzLang 0.1.0
```

---

## Tests

TzLang cuenta actualmente con **138 pruebas automatizadas**.

Ejecutar la suite:

```bash
make test
```

Resultado esperado:

```
========================================
Tests:  138
Passed: 138
Failed: 0
========================================
All tests passed.
```

Las pruebas cubren:

- Aritmética
- Conversión de tipos
- Decimales
- Overflow
- Menos unario
- Built-ins
- Strings
- Listas
- Diccionarios
- Indexación
- Control de flujo
- Funciones
- Recursión
- Scope
- Errores del lexer
- Errores del parser
- Errores del interpreter
- CLI
- Archivos
- BOM UTF-8
- CRLF
- Archivos vacíos
- Ejemplos completos

---

## Debug

Para generar una versión de depuración:

```bash
make debug
```

Genera:

```
build/tzc-debug
```

Ejecutar:

```bash
./build/tzc-debug examples/hola.tz
```

---

## AddressSanitizer y UBSan

TzLang puede compilarse con sanitizers:

```bash
make asan
```

Esto genera:

```
build/tzc-asan
```

y ejecuta automáticamente la suite de pruebas.

Se utilizan:

```
AddressSanitizer
UndefinedBehaviorSanitizer
```

para detectar problemas como:

- Use-after-free
- Double-free
- Buffer overflow
- Undefined behavior
- Problemas de memoria

---

## Limpiar el proyecto

```bash
make clean
```

Esto elimina los artefactos generados dentro de:

```
build/
```

Los binarios y archivos objeto no forman parte del repositorio.

---

## Códigos de salida

El CLI utiliza los siguientes códigos:

| Código | Significado |
|--------|-------------|
| `0` | Ejecución correcta |
| `1` | Error de argumentos u opciones |
| `2` | Error de archivo o extensión |
| `3` | Error de lexer, parser o interpreter |

---

## Filosofía del proyecto

TzLang busca mantener una implementación pequeña y comprensible.

Algunas decisiones actuales son deliberadamente simples.

Por ejemplo, los diccionarios utilizan un array dinámico de pares:

```
Dictionary
├── key
├── value
├── key
├── value
└── ...
```

La búsqueda es lineal y conserva el orden de inserción.

Para el tamaño esperado de los programas educativos actuales, esta solución prioriza:

- Simplicidad
- Código fácil de leer
- Comportamiento determinista
- Facilidad de depuración
- Gestión de memoria explícita

---

## Gestión de memoria

TzLang utiliza gestión explícita de memoria en C.

Los valores estructurados utilizan copia profunda.

Por ejemplo:

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

El resultado es:

```
20
```

La modificación de `b` no modifica `a`.

La implementación ha sido probada con AddressSanitizer, UndefinedBehaviorSanitizer y herramientas de detección de fugas durante el desarrollo.

---

## Limitaciones actuales

TzLang todavía está en desarrollo.

Actualmente no incluye:

- Unicode completo
- Sistema de módulos
- Clases
- Lambdas
- Generadores
- Sets
- Tuplas
- Garbage Collector
- Enteros grandes

Los textos actualmente tienen algunas limitaciones relacionadas con Unicode y `largo()` cuenta bytes en lugar de caracteres.

---

## Roadmap

### TzLang 0.1

- Lexer
- Parser
- AST
- Interpreter
- Runtime
- Variables
- Control de flujo
- Funciones
- Recursión
- Listas
- Diccionarios
- Built-ins
- CLI
- Sistema de tests
- Makefile
- Debug build
- ASan / UBSan

### Próximas versiones

- Mejor soporte Unicode
- Sistema de módulos
- Mejor diagnóstico de errores
- Documentación ampliada
- CMake
- Compatibilidad Windows
- Compatibilidad Linux/GCC

---

## Documentación

La referencia completa del lenguaje está disponible en:

```
docs/language.md
```

Ahí se documentan:

- Sintaxis
- Tipos
- Operadores
- Control de flujo
- Funciones
- Listas
- Diccionarios
- Built-ins
- CLI
- Arquitectura
- Limitaciones

---

## Estado

**TzLang 0.1.0**

Proyecto en desarrollo.

La implementación actual cuenta con:

```
138 tests
138 passed
0 failed
```

---

## Licencia

Este proyecto todavía no tiene una licencia pública definida.
