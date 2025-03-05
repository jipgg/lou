
add_executable(lou_framework WIN32
    lou_init.cpp
    lou_render.cpp
    lou_update.cpp
    luau_init.cpp
    meta_implementations.cpp
)
target_link_libraries(lou_framework PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_image::SDL3_image
    Luau.VM
    Luau.Common
    Luau.Config
    Luau.CodeGen
    Luau.CLI.lib
    Luau.Compiler
    imgui
    blaze
)
set_target_properties(lou_framework PROPERTIES OUTPUT_NAME lou)
