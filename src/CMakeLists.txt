cmake_minimum_required(VERSION 3.12)

project(chip8-interpreter)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


if (CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release)
    set(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} CACHE STRING "Configs" FORCE)
endif()


set(LIB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(BIN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIRECTORY})
foreach(OUTPUT_CONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string(TOUPPER ${OUTPUT_CONFIG} OUTPUT_CONFIG )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUT_CONFIG} ${LIB_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUT_CONFIG} ${LIB_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUT_CONFIG} ${BIN_OUTPUT_DIRECTORY})
endforeach(OUTPUT_CONFIG CMAKE_CONFIGURATION_TYPES)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()


# find dependencies
find_package(imgui CONFIG REQUIRED)
if(NOT imgui_FOUND)
    message(FATAL_ERROR "imgui not found")
endif()

find_package(gl3w CONFIG REQUIRED)
if(NOT gl3w_FOUND)
    message(FATAL_ERROR "gl3w not found")
endif()

find_path(MSGSL_INCLUDE_DIR gsl/gsl)
if (MSGSL_INCLUDE_DIR STREQUAL MSGSL_INCLUDE_DIR-NOTFOUND)
    message(FATAL_ERROR "MSGSL not found")
endif()

find_package(SDL2 CONFIG REQUIRED)
if(NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found")
endif()

find_path(TCLAP_INCLUDE_DIR tclap/CmdLine.h)
if (TCLAP_INCLUDE_DIR STREQUAL TCLAP_INCLUDE_DIR-NOTFOUND)
    message(FATAL_ERROR "TCLAP not found")
endif()

find_package(doctest CONFIG REQUIRED)
if(NOT doctest_FOUND)
    message(FATAL_ERROR "doctest not found")
endif()


if(MSVC)
    add_compile_options(/W4 /WX "$<IF:$<CONFIG:Debug>,/MTd,/MT>")
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()


add_subdirectory(core)
add_subdirectory(application)
