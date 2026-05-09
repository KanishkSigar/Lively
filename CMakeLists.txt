cmake_minimum_required(VERSION 3.25)
project(LiveLy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(src)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(lively ${SOURCES})

include(CTest)

add_executable(lively_vm_tests
    tests/vm_tests.cpp
    src/vm/vm.cpp
)

target_include_directories(lively_vm_tests PRIVATE src)

add_test(
    NAME vm_runtime
    COMMAND lively_vm_tests
)

# ---- Test 1: Lexer smoke-test (token stream) ----
add_test(
    NAME lexer_hello
    COMMAND lively ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello.lv
)

set_tests_properties(lexer_hello PROPERTIES
    PASS_REGULAR_EXPRESSION "BIND \\(bind\\).*TYPE_INT \\(int\\).*ASSIGN \\(is\\)"
)

# ---- Test 2: Parser smoke-test (AST generation) ----
add_test(
    NAME parser_hello
    COMMAND lively ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello.lv
)

set_tests_properties(parser_hello PROPERTIES
    PASS_REGULAR_EXPRESSION "ABSTRACT SYNTAX TREE.*VarDecl.*All compiler phases completed"
)

# ---- Test 3: Semantic analysis smoke-test ----
add_test(
    NAME semantic_hello
    COMMAND lively ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello.lv
)

set_tests_properties(semantic_hello PROPERTIES
    PASS_REGULAR_EXPRESSION "Semantic analysis passed.*All compiler phases completed"
)

# ---- Test 4: Negative — type mismatch should fail ----
add_test(
    NAME semantic_type_mismatch
    COMMAND lively ${CMAKE_CURRENT_SOURCE_DIR}/examples/errors/type_mismatch.lv
)

set_tests_properties(semantic_type_mismatch PROPERTIES
    PASS_REGULAR_EXPRESSION "Semantic Error.*Type mismatch"
)

# ---- Test 5: Negative — undefined variable should fail ----
add_test(
    NAME semantic_undefined
    COMMAND lively ${CMAKE_CURRENT_SOURCE_DIR}/examples/errors/undeclared.lv
)

set_tests_properties(semantic_undefined PROPERTIES
    PASS_REGULAR_EXPRESSION "Semantic Error.*Undefined variable"
)

# ---- Test 6: TAC generation smoke-test ----
add_test(
    NAME tac_hello
    COMMAND lively ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello.lv
)

set_tests_properties(tac_hello PROPERTIES
    PASS_REGULAR_EXPRESSION "THREE-ADDRESS CODE.*ASSIGN.*EMIT"
)

# ---- Test 7: TAC loop control-flow ----
add_test(
    NAME tac_loop
    COMMAND lively ${CMAKE_CURRENT_SOURCE_DIR}/examples/loop_test.lv
)

set_tests_properties(tac_loop PROPERTIES
    PASS_REGULAR_EXPRESSION "THREE-ADDRESS CODE.*LABEL.*IF_FALSE.*GOTO"
)

# ---- Test 8: Bytecode generation smoke-test ----
add_test(
    NAME bytecode_hello
    COMMAND lively ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello.lv
)

set_tests_properties(bytecode_hello PROPERTIES
    PASS_REGULAR_EXPRESSION "BYTECODE.*PUSH_CONST.*STORE.*PRINT"
)

# ---- Test 9: Bytecode loop jumps ----
add_test(
    NAME bytecode_loop
    COMMAND lively ${CMAKE_CURRENT_SOURCE_DIR}/examples/loop_test.lv
)

set_tests_properties(bytecode_loop PROPERTIES
    PASS_REGULAR_EXPRESSION "BYTECODE.*JUMP_IF_FALSE.*JUMP"
)

# ---- Convenience: run hello example manually ----
add_custom_target(run_hello
    COMMAND lively ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello.lv
    DEPENDS lively
    COMMENT "Running lively on examples/hello.lv"
)

# ---- Convenience: run all CTest tests ----
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS lively
    COMMENT "Running all CTest tests"
)