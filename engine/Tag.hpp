#pragma once
#include <SDL3/SDL.h>
#include "engine.hpp"

#define Tag_Enum_Item_Generator(X) \
    X(Any)\
    X(Engine)\
    X(Console)\
    X(Rect)\
    X(Point)\
    X(Color)\
    X(Texture)\
    X(Window)\
    X(Renderer)\
    X(COMPILE_TIME_ENUM_SENTINEL)

enum class Tag {
    #define Generate_Tag_Enum(item) item,
    Tag_Enum_Item_Generator(Generate_Tag_Enum)
    #undef Generate_Tag_Enum
};

enum class Tag_Reference {
    #define Generate_Tag_Reference_Enum(item) item = LUA_UTAG_LIMIT - static_cast<int>(Tag::item),
    Tag_Enum_Item_Generator(Generate_Tag_Reference_Enum)
    #undef Generate_Tag_Reference_Enum
};
#undef Tag_Enum_Item_Generator

template <Tag Tag> struct Mapped_Type;

#define Map_Type_To_Tag(TAG, TYPE)\
template <> struct Mapped_Type<Tag::TAG> {using Type = TYPE;}

Map_Type_To_Tag(Point, SDL_Point);
Map_Type_To_Tag(Rect, SDL_Rect);
Map_Type_To_Tag(Color, SDL_Color);
Map_Type_To_Tag(Engine, Engine);
Map_Type_To_Tag(Console, Console);

#undef Map_Type_To_Tag

template <Tag_Reference Val, typename As = Tag>
requires std::is_integral_v<As>
consteval auto tag_cast() -> Tag {
    return static_cast<As>(static_cast<int>(Val) - LUA_UTAG_LIMIT);
}
template <Tag Val, typename As = Tag_Reference>
requires std::is_integral_v<As>
consteval auto tag_reference_cast() -> Tag_Reference {
    return static_cast<As>(LUA_UTAG_LIMIT - static_cast<int>(Val));
}
template <Tag Val>
auto get_metatable_name() -> const char* {
    static const std::string name{compile_time::enum_info<Val>().raw};
    return name.c_str();
}
template <Tag_Reference Val>
auto get_metatable_name() -> decltype(auto) {
    return get_metatable_name<tag_cast<Val>()>();
}
template <Tag Val>
auto push_metatable(lua_State* L) -> void {
    luaL_getmetatable(L, get_metatable_name<Val>());
}
template <Tag_Reference Val>
auto push_metatable(lua_State *L) -> decltype(auto) {
    return push_metatable<tag_cast<Val>()>();
}
template <Tag Val, class Ty = Mapped_Type<Val>::Type>
auto new_object(lua_State* L) -> Ty& {
    auto& p = *static_cast<Ty*>(
        lua_newuserdatatagged(L, sizeof(Ty), static_cast<int>(Val))
    );
    new (p) Ty{};
    push_metatable<Val>(L);
    lua_setmetatable(L, -2);
    return p;
}
template <Tag Val, class Ty = Mapped_Type<Val>::Type>
auto push_reference(lua_State* L, Ty& ref) -> void {
    auto& p = *static_cast<Ty**>(
        lua_newuserdatatagged(L, sizeof(Ty*), tag_reference_cast<Val, int>())
    );
    p = &ref;
    push_metatable<Val>(L);
    lua_setmetatable(L, -2);
}
template <Tag Val, class Ty = Mapped_Type<Val>::Type>
constexpr auto is_tagged(lua_State* L, int idx) -> bool {
    const int tag = lua_userdatatag(L, idx);
    return tag == static_cast<int>(Val)
        or tag == tag_reference_cast<Val, int>();
}

template <Tag Val, class Ty = Mapped_Type<Val>::Type>
constexpr auto to_object(lua_State* L, int idx) -> Ty& {
    const int tag = lua_userdatatag(L, idx);
    if (tag == static_cast<int>(Val)) {
        return *static_cast<Ty*>(lua_touserdatatagged(L, idx, static_cast<int>(Val)));
    } else if (tag == tag_reference_cast<Val, int>()) {
        return *static_cast<Ty**>(lua_touserdatatagged(L, idx, tag_reference_cast<Val, int>()));
    }
    logger.log("Invalid cast. tag: {}", tag);
    return nullptr;
}

