#pragma once
#include <lualib.h>
#include <chrono>
#include <source_location>
#include <format>
#include <filesystem>
#include <list>
#include <fstream>
#include <source_location>
#include <string>
#include <type_traits>
#include <string_view>
#include <array>
#include <cassert>
#include <ranges>

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

struct Logger {
    std::ofstream file{"lou.log", std::ios::app};
    template <class ...Ts>
    auto log(const std::format_string<Ts...>& fmt, Ts&&...args) -> void {
        file << stamp_debug_info();
        file << std::format(fmt, std::forward<Ts>(args)...) << "\n";
        file.flush();
    }
};
inline Logger logger{};

// compile time utility
namespace compile_time {
template<class Val, Val v>
struct Value {
    static const constexpr auto value = v;
    constexpr operator Val() const {return v;}
};
template <class Func, std::size_t... Indices>
constexpr void unroll_for(Func fn, std::index_sequence<Indices...>) {
  (fn(Value<std::size_t, Indices>{}), ...);
}
template <std::size_t Count, typename Func>
constexpr void unroll_for(Func fn) {
  unroll_for(fn, std::make_index_sequence<Count>());
}
#define COMPILE_TIME_ENUM_SENTINEL _end
template <class Ty>
concept Sentinel_Enum = requires {
    std::is_enum_v<Ty>;
    Ty::COMPILE_TIME_ENUM_SENTINEL;
};
template <class From, class To>
concept Static_Castable_To = requires {
    static_cast<To>(From{});
};
template <Sentinel_Enum Type>
consteval std::size_t count() {
    return static_cast<std::size_t>(Type::COMPILE_TIME_ENUM_SENTINEL);
}
template <Sentinel_Enum Type, Type Last_Item>
consteval std::size_t count() {
    return static_cast<std::size_t>(Last_Item) + 1;
}
struct Enum_Info {
    std::string_view type;
    std::string_view name;
    std::string_view raw;
    int value;
};
template <Sentinel_Enum Type>
struct Enum_Item {
    std::string_view name;
    int value;
    constexpr Type to_enum() const {return Type(value);}
    constexpr operator Type() const {return Type(value);}
};
namespace detail {
#if defined (_MSC_VER)//msvc compiler
template <Sentinel_Enum Ty, Ty Val>
consteval Enum_Info enum_info() {
    const std::string_view raw{std::source_location::current().function_name()};
    const std::string enum_t_keyw{"<enum "};
    auto found = raw.find(enum_t_keyw);
    std::string_view enum_t = raw.substr(raw.find(enum_t_keyw) + enum_t_keyw.size());
    bool is_enum_class = enum_t.find("::") != std::string::npos;
    enum_t = enum_t.substr(0, enum_t.find_first_of(','));
    const std::string preceding = enum_t_keyw + std::string(enum_t) + ",";
    std::string_view enum_v = raw.substr(raw.find(preceding) + preceding.size());
    if (enum_v.find("::")) {
        enum_v = enum_v.substr(enum_v.find_last_of("::") + 1);
    }
    enum_v = enum_v.substr(0, enum_v.find_first_of(">"));
    return {
        .type = enum_t,
        .name = enum_v,
        .raw = raw,
        .value = static_cast<int>(Val),
    };
}
#elif defined(__clang__) || defined(__GNUC__)
template <Sentinel_Enum Ty, Ty Val>
consteval Enum_info enum_info() {
    using sv = std::string_view;
    const sv raw{std::source_location::current().function_name()};
    const sv enum_find{"Ty = "}; 
    const sv value_find{"Val = "};
    sv enum_t = raw.substr(raw.find(enum_find) + enum_find.size());
    enum_t = enum_t.substr(0, enum_t.find(";"));
    sv enum_v = raw.substr(raw.find(value_find) + value_find.size());
    if (enum_v.find("::")) {
        enum_v = enum_v.substr(enum_v.find_last_of("::") + 1);
    }
    enum_v = enum_v.substr(0, enum_v.find("]"));
    return {
        .type = enum_t,
        .name = enum_v,
        .raw = raw,
        .index = static_cast<int>(Val),
    };
}
#else
static_assert(false, "platform not supported")
#endif
}
template <auto Val, Sentinel_Enum Ty = decltype(Val)>
consteval Enum_Info enum_info() {
    return detail::enum_info<Ty, static_cast<Ty>(Val)>();
}
template <auto Val, Sentinel_Enum Ty = decltype(Val)>
constexpr Enum_Item<Ty> enum_item() {
    constexpr auto info = enum_info<Val, Ty>();
    return Enum_Item<Ty>{
        .name = info.name,
        .value = static_cast<int>(Val),
    };
}
template <Sentinel_Enum Ty, int Count = count<Ty>()> 
consteval std::array<Enum_Info, Count> to_array() {
    std::array<Enum_Info, Count> arr{};
    unroll_for<Count>([&arr](auto i) {
        arr[i] = enum_info<static_cast<Ty>(i.value)>();
    });
    return arr;
}
template <Sentinel_Enum Type, int Size = count<Type>()>
constexpr Enum_Item<Type> enum_item(std::string_view name) {
    constexpr auto array = to_array<Type, Size>();
    auto found_it = std::ranges::find_if(array, [&name](const Enum_Info& e) {return e.name == name;});
    assert(found_it != std::ranges::end(array));
    return {
        .name = found_it->name,
        .value = found_it->value
    };
}
template <Sentinel_Enum Type, int Size = count<Type>()>
constexpr Enum_Item<Type> enum_item(auto value) {
    constexpr auto array = to_array<Type, Size>();
    const auto& info = array.at(static_cast<size_t>(value)); 
    return {
        .name = info.name,
        .value = info.value
    };
}
}
// genenric lua utility
namespace lua {
class Ref {
    int ref_;
    lua_State* state_;
public:
    Ref(): ref_(-1), state_(nullptr) {}
    Ref(lua_State* L, int idx):
        ref_(lua_ref(L, idx)),
        state_(lua_mainthread(L)) {
    }
    Ref(const Ref& other) {
        state_ = other.state_;
        if (state_) {
            push(state_);
            ref_ = lua_ref(state_, -1);
            lua_pop(state_, 1);
        }
    }
    Ref(Ref&& other) noexcept {
        state_ = other.state_;
        ref_ = other.ref_;
        other.state_ = nullptr;
    }
    Ref& operator=(const Ref& other) {
        state_ = other.state_;
        if (state_) {
            push(state_);
            ref_ = lua_ref(state_, -1);
            lua_pop(state_, 1);
        }
        return *this;
    }
    Ref& operator=(Ref&& other) noexcept {
        state_ = other.state_;
        ref_ = other.ref_;
        other.state_ = nullptr;
        return *this;
    }
    ~Ref() {
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

template <class As = int>
auto namecall_atom(lua_State* L) -> std::pair<As, std::string_view> {
    int atom;
    auto namecall = lua_namecallatom(L, &atom);
    return {static_cast<As>(atom), namecall};
}
template<class Ty>
inline auto check_vector(lua_State* L, int idx) -> std::span<const float> {
    return std::span<const float>{luaL_checkvector(L, idx), LUA_VECTOR_SIZE};
}
inline auto to_vector(lua_State* L, int idx) -> std::span<const float> {
    return std::span<const float>{lua_tovector(L, idx), LUA_VECTOR_SIZE};
}
inline void push(lua_State* L, std::span<const float> vector) {
    lua_pushvector(L, vector[0], vector[1], vector[2]);
} 
template <class ...Tys>
[[noreturn]] constexpr void error(lua_State* L, const std::format_string<Tys...>& fmt, Tys&&...args) {
    luaL_errorL(L, std::format(fmt, std::forward<Tys>(args)...).c_str());
}
[[noreturn]] inline void error(lua_State* L, const std::string& msg) {
    luaL_errorL(L, msg.c_str());
}
template <class ...Ts>
[[noreturn]] constexpr void arg_error(lua_State* L, int idx, const std::format_string<Ts...>& fmt, Ts&&...args) {
    luaL_argerror(L, idx, format(fmt, std::forward<Ts>(args)...).c_str());
}
[[noreturn]] inline void arg_error(lua_State* L, int idx, const std::string& msg) {
    luaL_argerrorL(L, idx, msg.c_str());
}
template <class ...Ts>
[[noreturn]] constexpr void type_error(lua_State* L, int idx, const std::format_string<Ts...>& fmt, Ts&&...args) {
    luaL_typeerrorL(L, idx, format(fmt, std::forward<Ts>(args)...).c_str());
}
[[noreturn]] inline void type_error(lua_State* L, int idx, const std::string& msg) {
    luaL_typeerrorL(L, idx, msg.c_str());
}
[[nodiscard]] inline std::string_view tostring(lua_State* L, int idx) {
    size_t len;
    const char* str = luaL_tolstring(L, idx, &len);
    return {str, len};
}
template <class ...Ts>
void push(lua_State* L, const std::format_string<Ts...>& fmt, Ts&&...args) {
    lua_pushstring(L, std::format(fmt, std::forward<Ts>(args)...).c_str());
}
inline void push(lua_State* L, std::string_view string) {
    lua_pushlstring(L, string.data(), string.length());
}
inline void push(lua_State* L, const char* string) {
    lua_pushstring(L, string);
}
inline void push(lua_State* L, const std::string& string) {
    lua_pushlstring(L, string.data(), string.length());
}
inline void push(lua_State* L, const std::filesystem::path& string) {
    lua_pushstring(L, string.string().c_str());
}
inline void push(lua_State* L, double number) {
    lua_pushnumber(L, number);
}
inline void push(lua_State* L, int integer) {
    lua_pushinteger(L, integer);
}
inline void push(lua_State* L, bool boolean) {
    lua_pushboolean(L, boolean);
}
template <typename Ty>
concept Has_Push_Overloaded = requires (lua_State* L, Ty&& v) {
    push(L, std::forward<Ty>(v));
};
template <Has_Push_Overloaded ...Ty_Args>
int return_values(lua_State* L, Ty_Args...args) {
    (push(L, args), ...);
    return sizeof...(args);
} 
template <class Ty = void>
auto to_userdata_tagged(lua_State* L, int idx, int tag) -> Ty& {
    return *static_cast<Ty*>(lua_touserdatatagged(L, idx, tag));
}
template <class Ty = void>
auto to_userdata(lua_State* L, int idx) -> Ty& {
    return *static_cast<Ty*>(lua_touserdata(L, idx));
}
template <class Ty>
auto check_userdata_tagged(lua_State* L, int idx, int tag) -> Ty& {
    if (lua_userdatatag(L, idx) != tag) type_error(L, typeid(Ty).name());
    return *static_cast<Ty*>(lua_touserdatatagged(L, idx, tag));
}
template <class Ty>
concept Convertible_To_Int = requires(Ty&& v) {
    static_cast<int>(v);
};
template <class Ty, Convertible_To_Int Int>
auto new_userdata_tagged(lua_State* L, Int tag) -> Ty& {
    return *static_cast<Ty*>(
        lua_newuserdatatagged(L, sizeof(Ty), static_cast<int>(tag))
    );
}
template <class Ty>
void default_dtor(void* ud) {
    static_cast<Ty*>(ud)->~Ty();
}
template <class Ty>
auto new_userdata(lua_State* L, void(*dtor)(void*) = default_dtor<Ty>) -> Ty& {
    return *static_cast<Ty*>(lua_newuserdatadtor(L, sizeof(Ty), dtor));
}
template <class Ty, std::constructible_from<Ty> ...Params>
auto make_userdata(lua_State* L, Params&&...args) -> Ty& {
    Ty* ud = static_cast<Ty*>(lua_newuserdatadtor(L, sizeof(Ty), default_dtor<Ty>));
    new (ud) Ty{std::forward<Params>(args)...};
    return *ud;
}
/*
template <class Ty = double, double Default = 0>
requires Number_Compatible<Ty>
constexpr auto opt_number(lua_State* L, int idx) -> Ty {
    return static_cast<Ty>(luaL_optnumber(L, idx, Default));
}
*/
template <class Ty>
concept Number_Compatible = requires {
    {static_cast<Ty>(double{})} -> std::same_as<Ty>;
};
template <class Ty>
concept String_Compatible = requires (const Ty& str) {
    {str.data()} -> std::convertible_to<const char*>;
    {str.size()} -> std::convertible_to<size_t>;
};
template <class Ty>
constexpr auto to(lua_State* L, int idx) -> Ty {
    if constexpr (std::same_as<Ty, bool>) {
        return lua_toboolean(L, idx);
    } else if constexpr (std::same_as<Ty, char>) {
        return *lua_tostring(L, idx);
    } else if constexpr (Number_Compatible<Ty>) {
        return lua_tonumber(L, idx);
    } else if constexpr (String_Compatible<Ty>
        or std::same_as<Ty, const char*>) {
        return lua_tostring(L, idx);
    } else {
        static_assert(false, "unsupported type");
    } 
}
template <class Ty>
constexpr auto check(lua_State* L, int idx) -> Ty {
    if constexpr (std::same_as<Ty, bool>) {
        return luaL_checkboolean(L, idx);
    } else if constexpr (std::same_as<Ty, char>) {
        return *luaL_checkstring(L, idx);
    } else if constexpr (Number_Compatible<Ty>) {
        return luaL_checknumber(L, idx);
    } else if constexpr (String_Compatible<Ty>
        or std::same_as<Ty, const char*>) {
        return luaL_checkstring(L, idx);
    } else {
        static_assert(false, "unsupported type");
    } 
}
template <class ...Ty_Args>
constexpr auto check_args(lua_State* L, int start = 1) -> std::tuple<Ty_Args...> {
    auto make_tuple = [&L, &start]<size_t...Index>(std::index_sequence<Index...>) {
        return std::make_tuple<Ty_Args...>(check<Ty_Args>(L, Index + start)...);
    };
    return make_tuple(std::index_sequence_for<Ty_Args...>{});
}
template <class Ty> consteval auto default_value() -> decltype(auto) {
    if constexpr (std::same_as<Ty, bool>) {
        return false;
    } else if constexpr (Number_Compatible<Ty>) {
        return 0;
    } else if constexpr (String_Compatible<Ty>
        or std::same_as<Ty, const char*>) {
        return "";
    } else {
        static_assert(false, "unsupported type");
    } 
}
template <class Ty, auto Default = default_value<Ty>()>
constexpr auto opt(lua_State* L, int idx) -> Ty {
    if constexpr (std::same_as<Ty, bool>) {
        return luaL_optboolean(L, idx, Default);
    } else if constexpr (Number_Compatible<Ty>) {
        return luaL_optnumber(L, idx, Default);
    } else if constexpr (String_Compatible<Ty>
        or std::same_as<Ty, const char*>) {
        return luaL_optstring(L, idx, Default);
    } else {
        static_assert(false, "unsupported type");
    } 
}

inline auto tuple_tostring(lua_State* L, int startidx = 1) -> std::string {
    const int top = lua_gettop(L);
    std::string message;
    for (int i{startidx}; i <= top; ++i) {
        message.append(tostring(L, i)).append(", ");
    }
    message.pop_back();
    message.back() = '\n';
    return message;
}
template <class Ty>
concept Can_Output_Error = requires (lua_State* L) {
    Ty{}.error(lua_tostring(L, 1));
};

template <class Ty>
concept Callback_List_Add_Compatible = requires (lua_State* L) {
    push(L, Ty{});
} or std::is_void_v<Ty>;

template <Callback_List_Add_Compatible...Args>
struct Callback_List {
    std::list<Ref> handlers;
    void add(lua_State* L, int idx) {
        if (not lua_isfunction(L, idx)) lua::type_error(L, idx, "function");
        handlers.emplace_back(L, idx);
    }
    template <class Console_Like>
    requires Can_Output_Error<Console_Like>
    void call(lua_State* L, Console_Like& console, Args...args) {
        auto push_arg = [&L](auto arg) {push(L, std::forward<decltype(arg)>(arg));};
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
    std::list<Ref> handlers;
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
        if (not lua_isfunction(L, idx)) type_error(L, idx, "function");
        handlers.emplace_back(Ref(L, idx));
    }
};
}
