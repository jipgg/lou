
add_executable(lou_engine WIN32
    Point.cpp
    Color.cpp
    Rect.cpp
    init.cpp
    init_luau.cpp
    main.cpp
    meta.cpp
    render.cpp
    update.cpp
)
target_link_libraries(lou_engine PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    Luau.VM
    Luau.Common
    Luau.Config
    Luau.CodeGen
    Luau.CLI.lib
    Luau.Compiler
    imgui
)
set_target_properties(lou_engine PROPERTIES OUTPUT_NAME lou)
