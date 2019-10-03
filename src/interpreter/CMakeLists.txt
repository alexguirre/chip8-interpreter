cmake_minimum_required(VERSION 3.15)

if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

find_package(IMGUI REQUIRED)
if(NOT IMGUI_FOUND)
    message(ERROR "IMGUI not found")
endif()

find_package(GL3W REQUIRED)
if(NOT GL3W_FOUND)
    message(ERROR "GL3W not found")
endif()

find_path(MSGSL_INCLUDE_DIR gsl/gsl)
if (MSGSL_INCLUDE_DIR STREQUAL MSGSL_INCLUDE_DIR-NOTFOUND)
    message(ERROR "MSGSL not found")
else()
    include_directories(${MSGSL_INCLUDE_DIR})
endif()

find_package(SDL2 REQUIRED)
if(NOT SDL2_FOUND)
    message(ERROR "SDL2 not found")
endif()

find_path(TCLAP_INCLUDE_DIR gsl/gsl)
if (TCLAP_INCLUDE_DIR STREQUAL TCLAP_INCLUDE_DIR-NOTFOUND)
    message(ERROR "TCLAP not found")
else()
    include_directories(${TCLAP_INCLUDE_DIR})
endif()

add_executable(c8-interpreter
    "Display.cpp"
    "Display.h"
    "Icons.h"
    "ImGuiWindow.cpp"
    "ImGuiWindow.h"
    "Instructions.cpp"
    "Instructions.h"
    "Interpreter.cpp"
    "Interpreter.h"
    "InterpreterDebugger.cpp"
    "InterpreterDebugger.h"
    "Keyboard.cpp"
    "Keyboard.h"
    "main.cpp"
    "Resources.cpp"
    "Resources.h"
    "Sound.cpp"
    "Sound.h"
)

set_target_properties(c8-interpreter PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

add_dependencies(c8-interpreter
    imgui::imgui
    unofficial::gl3w::gl3w
    SDL2::SDL2
    SDL2::SDL2main
)

target_link_libraries(c8-interpreter PRIVATE
    imgui::imgui
    unofficial::gl3w::gl3w
    SDL2::SDL2
    SDL2::SDL2main
)

if(MSVC)
    target_compile_options(c8-interpreter PRIVATE /W4 /WX)
else()
    target_compile_options(c8-interpreter PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
