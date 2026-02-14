cmake_minimum_required(VERSION 3.10)
project(design_models)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(design_models INTERFACE)

target_include_directories(design_models INTERFACE include)

