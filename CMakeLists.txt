cmake_minimum_required(VERSION 3.20)
project(JasperCompiler)

set(CMAKE_CXX_STANDARD 20)

find_package(BISON 2.1.3)
find_package(FLEX 2.5.13)
find_package(LLVM REQUIRED CONFIG)

BISON_TARGET(JasperCompiler parser.yy ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/parser.hpp)
FLEX_TARGET(JasperCompiler scanner.ll ${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(JasperCompiler
    ${BISON_JasperCompiler_OUTPUTS}
    ${FLEX_JasperCompiler_OUTPUTS}
    main.cpp
    driver.cpp
    driver.hpp
    jasper_number.cpp
    jasper_number.hpp)

llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(JasperCompiler ${llvm_libs})