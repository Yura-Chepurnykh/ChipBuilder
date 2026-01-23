cmake_minimum_required(VERSION 3.10)

project(chip_builder)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-std=c++23)

file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/src/*.cpp"
)

# message(STATUS "Sources: ${SOURCES}")

add_executable(chip_builder main.cpp ${SOURCES})

target_include_directories(chip_builder PRIVATE 
    core/drc/include
    core/data_structures/include
    models/include
    utils/concepts/include
    cli/validation/include
    cli/parser/include
    cli/commands/include
    cli/view/include
    cli/controller/include
)
