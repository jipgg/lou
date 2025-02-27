#pragma once
#include <lualib.h>

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
    Lua_Ref(Lua_Ref&& other) {
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
        lua_getref(L, ref_);
    }
    void release() {
        state_ = nullptr;
    }
};
