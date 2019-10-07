cmake_minimum_required(VERSION 3.12)

add_executable(c8-application
    "AppPlatform.cpp"
    "AppPlatform.h"
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

target_include_directories(c8-application PRIVATE ${MSGSL_INCLUDE_DIR})
target_include_directories(c8-application PRIVATE ${TCLAP_INCLUDE_DIR})

get_target_property(CORE_INCLUDE_DIR c8-core SOURCE_DIR)
get_filename_component(CORE_INCLUDE_DIR ${CORE_INCLUDE_DIR} DIRECTORY)
if (CORE_INCLUDE_DIR STREQUAL CORE_INCLUDE_DIR-NOTFOUND)
    message(FATAL_ERROR "c8-core not found")
else()
    target_include_directories(c8-application PRIVATE ${CORE_INCLUDE_DIR})
endif()

set_target_properties(c8-application PROPERTIES
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
