cmake_minimum_required(VERSION 3.10)

project(chip_builder)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chip_builder main.cpp)

target_include_directories(chip_builder PRIVATE core/drc/include)
