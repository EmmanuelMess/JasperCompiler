cmake_minimum_required(VERSION 3.20)
project(Interpreter)
project(Tests)
project(Playground)

set(CMAKE_CXX_STANDARD 20)

find_package(LLVM REQUIRED CONFIG)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

link_libraries()
execute_process(COMMAND llvm-config-14 --libs OUTPUT_VARIABLE LIBS)
execute_process(COMMAND llvm-config-14 --system-libs OUTPUT_VARIABLE SYS_LIBS)
execute_process(COMMAND llvm-config-14 --ldflags OUTPUT_VARIABLE LDF)
message(STATUS "Found LLVM" ${LIBS})

string(STRIP ${LIBS} LIBS)
string(STRIP ${SYS_LIBS} SYS_LIBS)
string(STRIP ${LDF} LDF)

link_libraries(${LIBS} ${SYS_LIBS} ${LDF})

execute_process(COMMAND llvm-config-14 --cxxflags OUTPUT_VARIABLE CMAKE_CXX_FLAGS)
string(STRIP ${CMAKE_CXX_FLAGS} CMAKE_CXX_FLAGS)

string(APPEND CMAKE_CXX_FLAGS " -fexceptions")

set(COMMON
    src/algorithms/tarjan_solver.cpp
    src/algorithms/tarjan_solver.hpp
    src/algorithms/unification.cpp
    src/algorithms/unification.hpp
    src/log/basic_stream.cpp
    src/log/basic_stream.hpp
    src/log/log.cpp
    src/log/log.hpp
    src/log/stream.cpp
    src/log/stream.hpp
    src/utils/polymorphic_block_allocator.cpp
    src/utils/polymorphic_block_allocator.hpp
    src/utils/polymorphic_dumb_allocator.cpp
    src/utils/polymorphic_dumb_allocator.hpp
    src/utils/block_allocator.cpp
    src/utils/block_allocator.hpp
    src/utils/interned_string.cpp
    src/utils/interned_string.hpp
    src/utils/span.cpp
    src/utils/span.hpp
    src/utils/string_set.cpp
    src/utils/string_set.hpp
    src/utils/string_view.cpp
    src/utils/string_view.hpp
    src/ast.cpp
    src/ast.hpp
    src/compile_time_environment.cpp
    src/compile_time_environment.hpp
    src/compute_offsets.cpp
    src/compute_offsets.hpp
    src/cst.cpp
    src/cst.hpp
    src/ct_eval.cpp
    src/ct_eval.hpp
    src/error_report.cpp
    src/error_report.hpp
    src/lexer.cpp
    src/lexer.hpp
    src/match_identifiers.cpp
    src/match_identifiers.hpp
    src/metacheck.cpp
    src/metacheck.hpp
    src/meta_unifier.cpp
    src/meta_unifier.hpp
    src/parser.cpp
    src/parser.hpp
    src/source_location.cpp
    src/source_location.hpp
    src/symbol_table.cpp
    src/symbol_table.hpp
    src/token.cpp
    src/token.hpp
    src/typecheck.cpp
    src/typecheck.hpp
    src/typechecker.cpp
    src/typechecker.hpp
    src/typesystem.cpp
    src/typesystem.hpp)

set(INTERPRETER
    src/interpreter/error.cpp
    src/interpreter/error.hpp
    src/interpreter/eval.cpp
    src/interpreter/eval.hpp
    src/interpreter/execute.cpp
    src/interpreter/execute.hpp
    src/interpreter/exit_status_tag.hpp
    src/interpreter/garbage_collector.cpp
    src/interpreter/garbage_collector.hpp
    src/interpreter/gc_ptr.cpp
    src/interpreter/gc_ptr.hpp
    src/interpreter/gc_cell.cpp
    src/interpreter/gc_cell.hpp
    src/interpreter/interpreter.cpp
    src/interpreter/interpreter.hpp
    src/interpreter/native.cpp
    src/interpreter/native.hpp
    src/interpreter/stack.cpp
    src/interpreter/stack.hpp
    src/interpreter/utils.cpp
    src/interpreter/utils.hpp
    src/interpreter/value.cpp
    src/interpreter/value.hpp)

set(COMPILER
    src/compiler/compiler.cpp
    src/compiler/compiler.hpp
    src/compiler/error.cpp
    src/compiler/error.hpp
    src/compiler/compile.cpp
    src/compiler/compile.hpp
    src/compiler/execute.cpp
    src/compiler/execute.hpp
    src/compiler/exit_status_tag.hpp
    src/compiler/garbage_collector.cpp
    src/compiler/garbage_collector.hpp
    src/compiler/gc_ptr.cpp
    src/compiler/gc_ptr.hpp
    src/compiler/gc_cell.cpp
    src/compiler/gc_cell.hpp
    src/compiler/native.cpp
    src/compiler/native.hpp
    src/compiler/stack.cpp
    src/compiler/stack.hpp
    src/compiler/utils.cpp
    src/compiler/utils.hpp
    src/compiler/value.cpp
    src/compiler/value.hpp)

set(TEST
    src/test/main.cpp
    src/test/test_set.cpp
    src/test/test_set.hpp
    src/test/tester.cpp
    src/test/tester.hpp)

add_compile_options(-std=c++2a -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wnull-dereference -Weffc++)

add_executable(Interpreter
    src/interpreter/main.cpp
    ${COMMON}
    ${INTERPRETER})

add_executable(Compiler
    src/compiler/main.cpp
    ${COMMON}
    ${COMPILER})
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(Compiler ${llvm_libs})

add_executable(Tests
    src/test/main.cpp
    ${COMMON}
    ${INTERPRETER}
    ${TEST})
add_executable(Playground
    src/playground/main.cpp
    ${COMMON})