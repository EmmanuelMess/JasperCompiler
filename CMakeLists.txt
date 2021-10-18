cmake_minimum_required(VERSION 3.20)
project(JasperCompiler C)

set(CMAKE_C_STANDARD 11)

find_package(BISON 2.1.3)
find_package(FLEX 2.5.13)

BISON_TARGET(JasperCompiler expressions.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h)
FLEX_TARGET(JasperCompiler lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)

add_executable(JasperCompiler ${BISON_JasperCompiler_OUTPUTS} ${FLEX_JasperCompiler_OUTPUTS})
