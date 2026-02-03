cmake_minimum_required(VERSION 3.10)
project(particle_simulation LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/src/glad.c)

target_include_directories(glad PUBLIC ./third_party/glad/include)

add_executable(
    particle_simulation 
    src/glfw.cpp
    src/errors.cpp
    src/window.cpp
    main.cpp
)

target_include_directories(particle_simulation PUBLIC ./include)

target_link_libraries(
    particle_simulation
    PRIVATE
    glad
    glfw
    OpenGL::GL
)
