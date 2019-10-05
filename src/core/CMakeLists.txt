cmake_minimum_required(VERSION 3.12)

if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

find_path(MSGSL_INCLUDE_DIR gsl/gsl)
if (MSGSL_INCLUDE_DIR STREQUAL MSGSL_INCLUDE_DIR-NOTFOUND)
    message(ERROR "MSGSL not found")
else()
    include_directories(${MSGSL_INCLUDE_DIR})
endif()

add_library(c8-core STATIC
	"Constants.h"
    "Instructions.cpp"
    "Instructions.h"
    "Interpreter.cpp"
    "Interpreter.h"
)

set_target_properties(c8-core PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

if(MSVC)
    target_compile_options(c8-core PRIVATE /W4 /WX "$<IF:$<CONFIG:Debug>,/MTd,/MT>")
else()
    target_compile_options(c8-core PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
