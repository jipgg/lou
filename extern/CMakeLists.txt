
add_subdirectory(Luau)
add_subdirectory(SDL)
add_subdirectory(SDL_ttf)
add_subdirectory(SDL_image)
add_subdirectory(blaze)

target_compile_definitions(Luau.VM PUBLIC
    LUA_UTAG_LIMIT=255
    LUA_LUTAG_LIMIT=255
    LUA_VECTOR_SIZE=4
)

add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)
target_link_libraries(imgui PRIVATE SDL3::SDL3)
