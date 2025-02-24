cmake_minimum_required(VERSION 3.1)

project(luwuw LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_subdirectory(extern/Luau)
add_subdirectory(extern/SDL)
add_subdirectory(extern/SDL_ttf)

add_subdirectory(engine)
