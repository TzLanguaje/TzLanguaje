# ==========================
# TzLang - build multiplataforma
# ==========================
#
# El Makefile sigue siendo el build
# de desarrollo en Unix. CMake existe
# para lo que el Makefile no puede
# hacer: compilar en Windows (MSVC /
# MinGW) y producir los binarios de
# los releases.
#
#   cmake -B build-cmake -DCMAKE_BUILD_TYPE=Release
#   cmake --build build-cmake
#
# Instalar:
#
#   cmake --install build-cmake --prefix ~/.local

cmake_minimum_required(VERSION 3.15)

# La version vive en src/version.h y
# en ningun otro sitio. Aqui se lee,
# no se repite.

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h" TZ_VERSION_HEADER)
string(REGEX MATCH "#define TZLANG_VERSION \"([0-9]+\\.[0-9]+\\.[0-9]+)\""
       _tz_match "${TZ_VERSION_HEADER}")
set(TZLANG_VERSION "${CMAKE_MATCH_1}")

if(NOT TZLANG_VERSION)
    message(FATAL_ERROR "No se pudo leer TZLANG_VERSION de src/version.h")
endif()

project(tzlang
    VERSION ${TZLANG_VERSION}
    LANGUAGES C
    DESCRIPTION "Un lenguaje de programacion educativo en espanol"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# ==========================
# FUENTES
# ==========================
#
# La misma lista que SRC en el
# Makefile. Explicita a proposito:
# nada de GLOB.

add_executable(tz
    src/main.c
    src/io/file.c
    src/lexer/lexer.c
    src/parser/parser.c
    src/ast/ast.c
    src/interpreter/interpreter.c
    src/runtime/value.c
    src/runtime/operations.c
)

target_include_directories(tz PRIVATE src)

# ==========================
# FLAGS POR COMPILADOR
# ==========================

if(MSVC)
    # /utf8: los literales del codigo
    # llevan acentos y MSVC asume la
    # codepage del sistema si no se
    # le dice lo contrario.
    target_compile_options(tz PRIVATE /W4 /utf8)
    target_compile_definitions(tz PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(tz PRIVATE -Wall -Wextra)
endif()

# ==========================
# ENLAZADO ESTATICO (releases)
# ==========================
#
# Solo para los binarios que se
# publican: un ejecutable estatico en
# Linux corre en cualquier distro sin
# depender de la version de glibc del
# runner que lo compilo.

option(TZLANG_STATIC "Enlazar estaticamente (para releases)" OFF)

if(TZLANG_STATIC)
    if(MSVC)
        set_property(TARGET tz PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded")
        target_compile_options(tz PRIVATE /MT)
    elseif(NOT APPLE)
        # macOS no permite enlazar la
        # libc estaticamente; alli se
        # ignora a proposito.
        target_link_options(tz PRIVATE -static)
    endif()
endif()

# ==========================
# INSTALAR
# ==========================

include(GNUInstallDirs)

install(TARGETS tz RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES LICENSE README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR})

# ==========================
# PRUEBAS
# ==========================
#
# Reutiliza el runner de siempre
# apuntado al binario de CMake.
# En Windows no hay sh, asi que
# la suite solo se registra en Unix.

if(UNIX)
    enable_testing()
    add_test(
        NAME suite
        COMMAND ${CMAKE_COMMAND} -E env
                TZC=$<TARGET_FILE:tz>
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

message(STATUS "TzLang ${TZLANG_VERSION} (${CMAKE_BUILD_TYPE})")
