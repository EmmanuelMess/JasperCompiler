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

link_libraries()
execute_process(COMMAND llvm-config --libs OUTPUT_VARIABLE LIBS)
execute_process(COMMAND llvm-config --system-libs OUTPUT_VARIABLE SYS_LIBS)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LDF)
message(STATUS "Found LLVM" ${LIBS})

string(STRIP ${LIBS} LIBS)
string(STRIP ${SYS_LIBS} SYS_LIBS)
string(STRIP ${LDF} LDF)

link_libraries(${LIBS} ${SYS_LIBS} ${LDF})

execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE CMAKE_CXX_FLAGS)
string(STRIP ${CMAKE_CXX_FLAGS} CMAKE_CXX_FLAGS)

string(APPEND CMAKE_CXX_FLAGS " -fexceptions")

set(SOURCES ${BISON_JasperCompiler_OUTPUTS}
    ${FLEX_JasperCompiler_OUTPUTS}
    main.cpp
    driver.cpp
    driver.hpp
    jasper_number.cpp
    jasper_number.hpp
    jasper_function.cpp
    jasper_function.hpp)

add_executable(JasperCompiler ${SOURCES})

llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(JasperCompiler ${llvm_libs})

file(WRITE ${PROJECT_BINARY_DIR}/empty.cpp "")
add_executable(JasperCompilerTestExecutable ${PROJECT_BINARY_DIR}/empty.cpp)

target_link_libraries(JasperCompilerTestExecutable
    ${CMAKE_CURRENT_SOURCE_DIR}/bin/output.o)
