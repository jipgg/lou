#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <expected>
#include <string>
#include <lualib.h>


using Expect_void = std::expected<void, std::string>;
inline SDL_Window* Window{};
inline SDL_Renderer* Renderer{};
inline TTF_TextEngine* Text_engine{};
inline lua_State* Lua_state{};

void setupState(lua_State* L);
