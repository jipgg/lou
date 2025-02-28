#pragma once
#include <lualib.h>

struct Rect {
    static void push_constructor(lua_State* L);
    static void push_metatable(lua_State* L);
};
struct Color {
    static void push_constructor(lua_State* L);
    static void push_metatable(lua_State* L);
};
struct Point {
    static void push_constructor(lua_State* L);
    static void push_metatable(lua_State* L);
};
