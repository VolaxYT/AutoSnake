cmake_minimum_required(VERSION 3.20)
project(autosnake VERSION 1.0.0)
cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
set(GLFW_USE_STATIC_LIBS ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(autosnake
    src/main.cpp
    src/glad.c
)

target_include_directories(autosnake PRIVATE
    src
    dependencies
)

target_link_libraries(autosnake PRIVATE
    C:/msys64/ucrt64/lib/libglfw3.a
    OpenGL::GL
    glm::glm
    -lopengl32
)

add_custom_command(TARGET autosnake POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/shaders $<TARGET_FILE_DIR:autosnake>/shaders)