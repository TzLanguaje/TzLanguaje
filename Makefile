# ==========================
# TzLang
# ==========================

CC      = clang
CFLAGS  = -Wall -Wextra -std=c11

SRC = \
	src/main.c \
	src/io/file.c \
	src/lexer/lexer.c \
	src/parser/parser.c \
	src/ast/ast.c \
	src/interpreter/interpreter.c \
	src/runtime/value.c \
	src/runtime/operations.c

TARGET = build/tzc

RUNNER = tests/run_tests.sh

# ==========================
# COMPILAR
# ==========================

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

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
# LIMPIAR
# ==========================

clean:
	rm -f $(TARGET)

.PHONY: all test clean
