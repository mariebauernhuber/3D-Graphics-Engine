cmake_minimum_required(VERSION 3.10)
project(RIPWAKE C)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLAD_DIR ${CMAKE_SOURCE_DIR}/deps/glad)
add_library(glad ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)
find_package(OpenGL REQUIRED)
find_package(GLEW)
find_package(glfw3 3.4 REQUIRED)

file(GLOB_RECURSE SRC_FILES src/*.c)

add_executable(RIPWAKE ${SRC_FILES})

target_include_directories(RIPWAKE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(RIPWAKE PRIVATE ${SDL3_INCLUDE_DIRS})
target_link_libraries(RIPWAKE ${SDL3_LIBRARIES})
target_link_libraries(RIPWAKE glfw)
target_link_libraries(RIPWAKE GLEW)
target_link_libraries(RIPWAKE OpenGL::GL)
target_link_libraries(RIPWAKE glad)

add_custom_target(run
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	COMMAND $<TARGET_FILE:RIPWAKE>
	DEPENDS RIPWAKE 
)
