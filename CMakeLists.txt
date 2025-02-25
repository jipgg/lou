cmake_minimum_required(VERSION 3.15)
project(luwuw LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# dependencies
add_subdirectory(extern)

# project
add_subdirectory(engine)
