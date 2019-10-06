cmake_minimum_required(VERSION 3.12)

if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

set(CORE_SOURCES
    "Constants.h"
    "Context.cpp"
    "Context.h"
    "Instructions.cpp"
    "Instructions.h"
    "Interpreter.cpp"
    "Interpreter.h"
    "Platform.h"
)

add_library(c8-core STATIC
    ${CORE_SOURCES}
)

target_compile_definitions(c8-core PRIVATE DOCTEST_CONFIG_DISABLE)

add_executable(c8-core-test
    ${CORE_SOURCES}
    "tests/main.cpp"
)

find_path(MSGSL_INCLUDE_DIR gsl/gsl)
if (MSGSL_INCLUDE_DIR STREQUAL MSGSL_INCLUDE_DIR-NOTFOUND)
    message(ERROR "MSGSL not found")
else()
    target_include_directories(c8-core PUBLIC ${MSGSL_INCLUDE_DIR})
    target_include_directories(c8-core-test PUBLIC ${MSGSL_INCLUDE_DIR})
endif()

find_package(doctest CONFIG REQUIRED)
if(NOT doctest_FOUND)
    message(ERROR "doctest not found")
endif()

set_target_properties(c8-core PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

set_target_properties(c8-core-test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

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

if(MSVC)
    target_compile_options(c8-core PRIVATE /W4 /WX "$<IF:$<CONFIG:Debug>,/MTd,/MT>")
    target_compile_options(c8-core-test PRIVATE /W4 /WX "$<IF:$<CONFIG:Debug>,/MTd,/MT>")
else()
    target_compile_options(c8-core PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(c8-core-test PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
