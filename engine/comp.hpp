#pragma once
#include <source_location>
#include <string>
#include <type_traits>
#include <string_view>
#include <array>
#include <cassert>
#include <ranges>
namespace comp {
template<class Val, Val v>
struct Value {
    static const constexpr auto value = v;
    constexpr operator Val() const {return v;}
};
template <class Func, std::size_t... indices>
constexpr void inline_for(Func fn, std::index_sequence<indices...>) {
  (fn(Value<std::size_t, indices>{}), ...);
}
template <std::size_t count, typename Func>
constexpr void inline_for(Func fn) {
  inline_for(fn, std::make_index_sequence<count>());
}
#define compenum_SENTINEL _end
template <class T>
concept Enum = std::is_enum_v<T>;
template <Enum Type>
consteval std::size_t count() {
    return static_cast<std::size_t>(Type::compenum_SENTINEL);
}
#define compenum_SENTINEL_ENUM(Enum_type, ...) enum class Enum_type {__VA_ARGS__, compenum_SENTINEL}

template <Enum Type, Type last_item>
consteval std::size_t count() {
    return static_cast<std::size_t>(last_item) + 1;
}
struct Enum_Info {
    std::string_view type;
    std::string_view name;
    std::string_view raw;
    int value;
};
template <Enum Type>
struct Enum_Item {
    std::string_view name;
    int value;
    constexpr Type to_enum() const {return Type(value);}
    constexpr operator Type() const {return Type(value);}
};
namespace detail {
#if defined (_MSC_VER)//msvc compiler
template <Enum Ty, Ty Val>
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
template <Enum Ty, Ty Val>
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
template <auto Val, Enum Ty = decltype(Val)>
consteval Enum_Info enum_info() {
    return detail::enum_info<Ty, static_cast<Ty>(Val)>();
}
template <auto Val, Enum Ty = decltype(Val)>
constexpr Enum_Item<Ty> enum_item() {
    constexpr auto info = enum_info<Ty, Val>();
    return Enum_Item<Ty>{
        .name = info.name,
        .value = static_cast<int>(Val),
    };
}
template <Enum Ty, int Count = count<Ty>()> 
consteval std::array<Enum_Info, Count> to_array() {
    std::array<Enum_Info, Count> arr{};
    inline_for<Count>([&arr](auto i) {
        arr[i] = enum_info<static_cast<Ty>(i.value)>();
    });
    return arr;
}
template <Enum Type, int size = count<Type>()>
constexpr Enum_Item<Type> enum_item(std::string_view name) {
    constexpr auto array = to_array<Type, size>();
    auto found_it = std::ranges::find_if(array, [&name](const Enum_Info& e) {return e.name == name;});
    assert(found_it != std::ranges::end(array));
    return {.name = found_it->name, .value = found_it->value};
}
template <Enum Type, int size = count<Type>()>
constexpr Enum_Item<Type> enum_item(int value) {
    constexpr auto array = to_array<Type, size>();
    auto found_it = std::ranges::find_if(array, [&value](const Enum_Info& e) {return e.value == value;});
    assert(found_it != std::ranges::end(array));
    return {.name = found_it->name, .value = found_it->value};
}
}
