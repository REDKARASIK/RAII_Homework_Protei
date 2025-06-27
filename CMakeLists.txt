cmake_minimum_required(VERSION 3.23)
project(
    FileRAII
    DESCRIPTION "Homework: make RAII File handler"
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(bin)
add_subdirectory(lib)