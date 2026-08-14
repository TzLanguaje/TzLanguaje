# ==========================
# TzLang
# ==========================
#
# Objetivos:
#
#   make / make all   compilar build/tzc
#   make test         compilar y ejecutar la suite
#   make debug        build/tzc-debug con -g -O0
#   make asan         build/tzc-asan con ASan+UBSan
#                     y ejecutar la suite con el
#   make clean        borrar todo lo generado
#
# El compilador se puede elegir:
#
#   make CC=clang
#   make CC=gcc

CC      ?= cc
CFLAGS  ?= -Wall -Wextra -std=c11

# Flags propios de cada variante.
# No ocultan warnings: siempre -Wall -Wextra.

DEBUG_FLAGS = -Wall -Wextra -std=c11 -g -O0
ASAN_FLAGS  = -Wall -Wextra -std=c11 -g -O1 \
              -fsanitize=address,undefined \
              -fno-omit-frame-pointer

SRC = \
	src/main.c \
	src/io/file.c \
	src/lexer/lexer.c \
	src/parser/parser.c \
	src/ast/ast.c \
	src/interpreter/interpreter.c \
	src/runtime/value.c \
	src/runtime/operations.c

OBJ = \
	build/main.o \
	build/io/file.o \
	build/lexer/lexer.o \
	build/parser/parser.o \
	build/ast/ast.o \
	build/interpreter/interpreter.o \
	build/runtime/value.o \
	build/runtime/operations.o

TARGET = build/tzc

RUNNER = tests/run_tests.sh

# ==========================
# COMPILAR
# ==========================

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

# ==========================
# OBJETOS
# ==========================
#
# Reglas explicitas, sin patrones de
# GNU make, para que el Makefile
# funcione igual en cualquier make.
#
# Cada objeto declara su .c y los
# headers que ese .c incluye de
# verdad (directa o indirectamente),
# de modo que tocar un header solo
# recompila a quien lo usa.

build/main.o: src/main.c src/version.h src/io/file.h \
              src/lexer/lexer.h src/parser/parser.h \
              src/ast/ast.h src/interpreter/interpreter.h \
              src/runtime/value.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

build/io/file.o: src/io/file.c src/io/file.h
	@mkdir -p build/io
	$(CC) $(CFLAGS) -c src/io/file.c -o build/io/file.o

build/lexer/lexer.o: src/lexer/lexer.c src/lexer/lexer.h
	@mkdir -p build/lexer
	$(CC) $(CFLAGS) -c src/lexer/lexer.c -o build/lexer/lexer.o

build/parser/parser.o: src/parser/parser.c src/parser/parser.h \
                       src/ast/ast.h src/lexer/lexer.h
	@mkdir -p build/parser
	$(CC) $(CFLAGS) -c src/parser/parser.c -o build/parser/parser.o

build/ast/ast.o: src/ast/ast.c src/ast/ast.h
	@mkdir -p build/ast
	$(CC) $(CFLAGS) -c src/ast/ast.c -o build/ast/ast.o

build/interpreter/interpreter.o: src/interpreter/interpreter.c \
                                 src/interpreter/interpreter.h \
                                 src/ast/ast.h src/runtime/value.h \
                                 src/runtime/operations.h
	@mkdir -p build/interpreter
	$(CC) $(CFLAGS) -c src/interpreter/interpreter.c -o build/interpreter/interpreter.o

build/runtime/value.o: src/runtime/value.c src/runtime/value.h
	@mkdir -p build/runtime
	$(CC) $(CFLAGS) -c src/runtime/value.c -o build/runtime/value.o

build/runtime/operations.o: src/runtime/operations.c \
                            src/runtime/operations.h \
                            src/runtime/value.h
	@mkdir -p build/runtime
	$(CC) $(CFLAGS) -c src/runtime/operations.c -o build/runtime/operations.o

# ==========================
# PROBAR
# ==========================
#
# Depende del binario, asi que
# 'make test' compila antes de
# probar. El runner NO compila.

test: $(TARGET)
	@$(RUNNER)

# ==========================
# DEBUG
# ==========================
#
# Binario APARTE (build/tzc-debug)
# con -g -O0, compilado en una sola
# pasada sin reutilizar los objetos
# normales para no mezclar flags.
#
# No toca build/tzc: las tres
# variantes conviven:
#
#   build/tzc         normal
#   build/tzc-debug   debug
#   build/tzc-asan    instrumentado

debug:
	@mkdir -p build
	$(CC) $(DEBUG_FLAGS) $(SRC) -o build/tzc-debug

# ==========================
# ASAN + UBSAN
# ==========================
#
# Binario aparte (build/tzc-asan)
# para no pisar el normal, y la
# MISMA suite de siempre apuntada a
# el mediante la variable TZC del
# runner. La suite no se duplica.

asan:
	@mkdir -p build
	$(CC) $(ASAN_FLAGS) $(SRC) -o build/tzc-asan
	TZC="`pwd`/build/tzc-asan" $(RUNNER)

# ==========================
# LIMPIAR
# ==========================
#
# build/ solo contiene artefactos
# generados: fuera entero.

clean:
	rm -rf build

.PHONY: all clean test debug asan
