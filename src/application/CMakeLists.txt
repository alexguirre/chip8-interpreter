cmake_minimum_required(VERSION 3.12)

if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

add_executable(c8-application
    "Display.cpp"
    "Display.h"
    "Icons.h"
    "ImGuiWindow.cpp"
    "ImGuiWindow.h"
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

find_package(imgui CONFIG REQUIRED)
if(NOT imgui_FOUND)
    message(ERROR "imgui not found")
endif()

find_package(gl3w CONFIG REQUIRED)
if(NOT gl3w_FOUND)
    message(ERROR "gl3w not found")
endif()

find_path(MSGSL_INCLUDE_DIR gsl/gsl)
if (MSGSL_INCLUDE_DIR STREQUAL MSGSL_INCLUDE_DIR-NOTFOUND)
    message(ERROR "MSGSL not found")
else()
    target_include_directories(c8-application PRIVATE ${MSGSL_INCLUDE_DIR})
endif()

find_package(SDL2 CONFIG REQUIRED)
if(NOT SDL2_FOUND)
    message(ERROR "SDL2 not found")
endif()

find_path(TCLAP_INCLUDE_DIR tclap/CmdLine.h)
if (TCLAP_INCLUDE_DIR STREQUAL TCLAP_INCLUDE_DIR-NOTFOUND)
    message(ERROR "TCLAP not found")
else()
    target_include_directories(c8-application PRIVATE ${TCLAP_INCLUDE_DIR})
endif()

get_target_property(CORE_INCLUDE_DIR c8-core SOURCE_DIR)
get_filename_component(CORE_INCLUDE_DIR ${CORE_INCLUDE_DIR} DIRECTORY)
if (CORE_INCLUDE_DIR STREQUAL CORE_INCLUDE_DIR-NOTFOUND)
    message(ERROR "c8-core not found")
else()
    target_include_directories(c8-application PRIVATE ${CORE_INCLUDE_DIR})
endif()

set_target_properties(c8-application PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME c8
)

add_dependencies(c8-application
    imgui::imgui
    unofficial::gl3w::gl3w
    SDL2::SDL2main
    SDL2::SDL2-static
)

target_link_libraries(c8-application PRIVATE
    imgui::imgui
    unofficial::gl3w::gl3w
    SDL2::SDL2main
    SDL2::SDL2-static
    c8-core
)

if(MSVC)
    target_compile_options(c8-application PRIVATE /W4 /WX "$<IF:$<CONFIG:Debug>,/MTd,/MT>")
else()
    target_compile_options(c8-application PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
