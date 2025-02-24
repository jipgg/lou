
file(GLOB luwuw_engine_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(luwuw_engine WIN32 ${luwuw_engine_sources})
target_link_libraries(luwuw_engine PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    Luau.VM
    Luau.Common
    Luau.Config
    Luau.CodeGen
    Luau.CLI.lib
    Luau.Compiler
)
