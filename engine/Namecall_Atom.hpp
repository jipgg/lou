#pragma once
#include "compile_time.hpp"


enum class Namecall_Atom: int16_t {
    print,
    comment,
    error,
    warn,
    get_console,
    get_window,
    get_renderer,
    get_callbacks,
    on_update,
    on_draw,
    pressed,
    released,
    is_pressed,
    is_down,
    is_key_down,
    COMPILE_TIME_ENUM_SENTINEL
};
