cmake_minimum_required(VERSION 3.12)

set(CORE_SOURCES
    "Constants.h"
    "Context.cpp"
    "Context.h"
    "Instructions.cpp"
    "Instructions.h"
    "Interpreter.cpp"
    "Interpreter.h"
    "Platform.h"
    "TypeAliases.h"
)

add_library(c8-core STATIC
    ${CORE_SOURCES}
)

target_compile_definitions(c8-core PRIVATE DOCTEST_CONFIG_DISABLE)

add_executable(c8-core-test
    ${CORE_SOURCES}
    "tests/main.cpp"
)

target_include_directories(c8-core PUBLIC ${MSGSL_INCLUDE_DIR})
target_include_directories(c8-core-test PUBLIC ${MSGSL_INCLUDE_DIR})

add_dependencies(c8-core
    doctest::doctest
)

add_dependencies(c8-core-test
    doctest::doctest
)

target_link_libraries(c8-core PRIVATE
    doctest::doctest
)

target_link_libraries(c8-core-test PRIVATE
    doctest::doctest
)
