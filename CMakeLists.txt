cmake_minimum_required(VERSION 3.20)
project(JasperCompiler)

set(CMAKE_CXX_STANDARD 20)

find_package(BISON 2.1.3)
find_package(FLEX 2.5.13)

BISON_TARGET(JasperCompiler parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp)
FLEX_TARGET(JasperCompiler scanner.ll ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(main.cpp main.cpp COPYONLY)
configure_file(driver.cpp driver.cpp COPYONLY)
configure_file(driver.hpp driver.hpp COPYONLY)
configure_file(jasper_number.cpp jasper_number.cpp COPYONLY)
configure_file(jasper_number.hpp jasper_number.hpp COPYONLY)

add_executable(JasperCompiler
    ${BISON_JasperCompiler_OUTPUTS}
    ${FLEX_JasperCompiler_OUTPUTS}
    main.cpp
    driver.cpp
    driver.hpp
    jasper_number.cpp
    jasper_number.hpp)