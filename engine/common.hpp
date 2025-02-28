#pragma once
#include <lualib.h>
#include <chrono>
#include <source_location>
#include <format>
#include <filesystem>
#include "lua_util.hpp"
#include <list>

inline auto stamp_time() -> std::string {
    using namespace std::chrono;
    using Duration = duration<long long, std::centi>;
    auto now = zoned_time(current_zone(), time_point_cast<Duration>(system_clock::now()));
    return std::format("[{:%T}]: ", now);
}

inline auto stamp_debug_info(const std::source_location& location = std::source_location::current()) -> std::string {
    using namespace std::chrono;
    using namespace std::filesystem;
    using Duration = duration<long long, std::centi>;
    auto now = time_point_cast<Duration>(system_clock::now());
    return std::format("[{:%T}]({}:{}): ", now, path(location.file_name()).filename().string(), location.line());
}

class Lua_Ref {
    int ref_;
    lua_State* state_;
public:
    Lua_Ref(): ref_(-1), state_(nullptr) {}
    Lua_Ref(lua_State* L, int idx):
        ref_(lua_ref(L, idx)),
        state_(lua_mainthread(L)) {
    }
    Lua_Ref(const Lua_Ref& other) {
        state_ = other.state_;
        if (state_) {
            push(state_);
            ref_ = lua_ref(state_, -1);
            lua_pop(state_, 1);
        }
    }
    Lua_Ref(Lua_Ref&& other) noexcept {
        state_ = other.state_;
        ref_ = other.ref_;
        other.state_ = nullptr;
    }
    Lua_Ref& operator=(const Lua_Ref& other) {
        state_ = other.state_;
        if (state_) {
            push(state_);
            ref_ = lua_ref(state_, -1);
            lua_pop(state_, 1);
        }
        return *this;
    }
    Lua_Ref& operator=(Lua_Ref&& other) noexcept {
        state_ = other.state_;
        ref_ = other.ref_;
        other.state_ = nullptr;
        return *this;
    }
    ~Lua_Ref() {
        if (state_) {
            lua_unref(state_, ref_);
        }
    }
    operator bool() const {
        return state_;
    }
    void push(lua_State* L) {
        if (not state_) {
            lua_pushnil(L);
            return;
        }
        lua_getref(L, ref_);
    }
    void release() {
        state_ = nullptr;
    }
};
template <class Ty>
concept Can_Output_Error = requires (lua_State* L) {
    Ty{}.error(lua_tostring(L, 1));
};
template <class Ty>
concept Has_Push_Overloaded = requires (lua_State* L) {
    lua::push(L, Ty{});
} or std::is_void_v<Ty>;
template <Has_Push_Overloaded ...Args>
struct Callback_List {
    std::list<Lua_Ref> handlers;
    void add(lua_State* L, int idx) {
        if (not lua_isfunction(L, idx)) lua::type_error(L, idx, "function");
        handlers.emplace_back(Lua_Ref(L, idx));
    }
    template <class Console_Like>
    requires Can_Output_Error<Console_Like>
    void call(lua_State* L, Console_Like& console, Args...args) {
        auto push_arg = [&L](auto arg) {lua::push(L, std::forward<decltype(arg)>(arg));};
        for (auto& fn : handlers) {
            fn.push(L);
            (push_arg(std::forward<Args>(args)),...);
            if (lua_pcall(L, sizeof...(Args), 0, 0) != LUA_OK) {
                console.error(lua_tostring(L, 1));
                lua_pop(L, 1);
            }
        }
    }
};
template <>
struct Callback_List<void> {
    std::list<Lua_Ref> handlers;
    template <class Console_Like>
    requires Can_Output_Error<Console_Like>
    auto call(lua_State* L, Console_Like& console) {
        for (auto& fn : handlers) {
            fn.push(L);
            if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                console.error(lua_tostring(L, -1));
                lua_pop(L, 1);
            }
        }
    }
    void add(lua_State* L, int idx) {
        if (not lua_isfunction(L, idx)) lua::type_error(L, idx, "function");
        handlers.emplace_back(Lua_Ref(L, idx));
    }
};

