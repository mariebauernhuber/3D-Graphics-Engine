cmake_minimum_required(VERSION 3.20)
project(RIPWAKE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)  # Adjust if needed
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Global flags matching your CPPFLAGS
add_compile_options(-Wall -Wextra -g -fdiagnostics-color=always)

# Create build dir equivalent
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)

# OpenGL, GLEW, GLFW, Assimp (assume pkg-config or Find modules)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

# App sources
file(GLOB APP_SOURCES src/*.cpp)

# ImGui sources (explicit, matching your list)
set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_sdlrenderer3.cpp
)

find_package(Catch2 REQUIRED)
# Main target
add_executable(app ${APP_SOURCES} ${IMGUI_SOURCES})

target_include_directories(app PRIVATE
    include
    imgui
    imgui/backends
    ${SDL3_INCLUDE_DIRS}
)

target_link_libraries(app PRIVATE
    ${SDL3_LIBRARIES}
    OpenGL::GL
    GLEW::GLEW
    glfw
    assimp
    m  # -lm
)

target_link_options(app PRIVATE ${SDL3_LDFLAGS_OTHER})  # Any extra linker flags


add_custom_target(run
    COMMAND $<TARGET_FILE:app> ${ARGS}
    DEPENDS app
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(test-mode
    COMMAND $<TARGET_FILE:app> --test
    DEPENDS app
)

add_custom_target(test-verbose
    COMMAND $<TARGET_FILE:app> --test-verbose
    DEPENDS app
)

