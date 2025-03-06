// most of this source code is yanked from the Repl source in luau
#include "Lou.hpp"
#include <Luau/Compiler.h>
#include <Luau/CodeGen.h>
#include <Luau/Require.h>
#include <ranges>
#include <algorithm>
#include <fstream>
#include <print>
#include "common.hpp"
namespace fs = std::filesystem;
namespace rngs = std::ranges;

static bool codegen = false;

auto copts() -> Luau::CompileOptions {
    Luau::CompileOptions result = {};
    result.optimizationLevel = 2;
    result.debugLevel = 3;
    result.typeInfoLevel = 1;
    result.coverageLevel = 2;
    return result;
}

static auto lua_loadstring(lua_State* L) -> int {
    size_t l = 0;
    const char* s = luaL_checklstring(L, 1, &l);
    const char* chunkname = luaL_optstring(L, 2, s);

    lua_setsafeenv(L, LUA_ENVIRONINDEX, false);

    std::string bytecode = Luau::compile(std::string(s, l), copts());
    if (luau_load(L, chunkname, bytecode.data(), bytecode.size(), 0) == 0)
        return 1;

    lua_pushnil(L);
    lua_insert(L, -2); // put before error message
    return 2;          // return nil plus error message
}

static auto finishrequire(lua_State* L) -> int {
    if (lua_isstring(L, -1))
        lua_error(L);
    return 1;
}

struct RuntimeRequireContext : public RequireResolver::RequireContext {
    explicit RuntimeRequireContext(std::string source): source(std::move(source)) {}

    std::string getPath() override {
        return source.substr(1);
    }
    bool isRequireAllowed() override {
        return isStdin() || (!source.empty() && source[0] == '@');
    }

    bool isStdin() override {
        return source == "=stdin";
    }
    std::string createNewIdentifer(const std::string& path) override {
        return "@" + path;
    }

private:
    std::string source;
};

struct RuntimeCacheManager : public RequireResolver::CacheManager
{
    explicit RuntimeCacheManager(lua_State* L): L(L) {}

    bool isCached(const std::string& path) override {
        luaL_findtable(L, LUA_REGISTRYINDEX, "_MODULES", 1);
        lua_getfield(L, -1, path.c_str());
        bool cached = !lua_isnil(L, -1);
        lua_pop(L, 2);

        if (cached)
            cacheKey = path;

        return cached;
    }

    std::string cacheKey;

private:
    lua_State* L;
};

struct RuntimeErrorHandler : RequireResolver::ErrorHandler
{
    explicit RuntimeErrorHandler(lua_State* L)
        : L(L)
    {
    }

    void reportError(const std::string message) override
    {
        luaL_errorL(L, "%s", message.c_str());
    }

private:
    lua_State* L;
};

static int lua_require(lua_State* L) {
    std::string name = luaL_checkstring(L, 1);

    RequireResolver::ResolvedRequire resolvedRequire;
    {
        lua_Debug ar;
        lua_getinfo(L, 1, "s", &ar);

        RuntimeRequireContext requireContext{ar.source};
        RuntimeCacheManager cacheManager{L};
        RuntimeErrorHandler errorHandler{L};

        RequireResolver resolver(std::move(name), requireContext, cacheManager, errorHandler);

        resolvedRequire = resolver.resolveRequire(
            [L, &cacheKey = cacheManager.cacheKey](const RequireResolver::ModuleStatus status)
            {
lua_getfield(L, LUA_REGISTRYINDEX, "_MODULES");
                if (status == RequireResolver::ModuleStatus::Cached)
                    lua_getfield(L, -1, cacheKey.c_str());
            }
        );
    }

    if (resolvedRequire.status == RequireResolver::ModuleStatus::Cached)
        return finishrequire(L);

    // module needs to run in a new thread, isolated from the rest
    // note: we create ML on main thread so that it doesn't inherit environment of L
    lua_State* GL = lua_mainthread(L);
    lua_State* ML = lua_newthread(GL);
    lua_xmove(GL, L, 1);

    // new thread needs to have the globals sandboxed
    luaL_sandboxthread(ML);

    // now we can compile & run module on the new thread
    std::string bytecode = Luau::compile(resolvedRequire.sourceCode, copts());
    if (luau_load(ML, resolvedRequire.identifier.c_str(), bytecode.data(), bytecode.size(), 0) == 0)
    {
        if (codegen)
        {
            Luau::CodeGen::CompilationOptions nativeOptions;
            Luau::CodeGen::compile(ML, -1, nativeOptions);
        }

        int status = lua_resume(ML, L, 0);

        if (status == 0)
        {
            if (lua_gettop(ML) == 0)
                lua_pushstring(ML, "module must return a value");
            else if (!lua_istable(ML, -1) && !lua_isfunction(ML, -1))
                lua_pushstring(ML, "module must return a table or function");
        }
        else if (status == LUA_YIELD)
        {
            lua_pushstring(ML, "module can not yield");
        }
        else if (!lua_isstring(ML, -1))
        {
            lua_pushstring(ML, "unknown error while running module");
        }
    }

    // there's now a return value on top of ML; L stack: _MODULES ML
    lua_xmove(ML, L, 1);
    lua_pushvalue(L, -1);
    lua_setfield(L, -4, resolvedRequire.absolutePath.c_str());

    // L stack: _MODULES ML result
    return finishrequire(L);
}

static int lua_collectgarbage(lua_State* L)
{
    const char* option = luaL_optstring(L, 1, "collect");

    if (strcmp(option, "collect") == 0)
    {
        lua_gc(L, LUA_GCCOLLECT, 0);
        return 0;
    }

    if (strcmp(option, "count") == 0)
    {
        int c = lua_gc(L, LUA_GCCOUNT, 0);
        lua_pushnumber(L, c);
        return 1;
    }

    luaL_error(L, "collectgarbage must be called with 'count' or 'collect'");
}

#ifdef CALLGRIND
static int lua_callgrind(lua_State* L)
{
    const char* option = luaL_checkstring(L, 1);

    if (strcmp(option, "running") == 0)
    {
        int r = RUNNING_ON_VALGRIND;
        lua_pushboolean(L, r);
        return 1;
    }

    if (strcmp(option, "zero") == 0)
    {
        CALLGRIND_ZERO_STATS;
        return 0;
    }

    if (strcmp(option, "dump") == 0)
    {
        const char* name = luaL_checkstring(L, 2);

        CALLGRIND_DUMP_STATS_AT(name);
        return 0;
    }

    luaL_error(L, "callgrind must be called with one of 'running', 'zero', 'dump'");
}
#endif

static auto load_script(lua_State* L, const fs::path& path) -> std::expected<decltype(L), std::string> {
    auto mainThread = lua_mainthread(L);
    auto scriptThread = lua_newthread(mainThread);
    std::ifstream file{path};
    if (!file.is_open()) {
        return std::unexpected(std::format("failed to open {}", path.string()));
    }
    std::string line, contents;
    while (std::getline(file, line)) contents.append(line + '\n');
    auto bytecode = Luau::compile(contents, copts());
    auto chunkname = std::format("=script:{}:", fs::relative(path).string());
    auto status = luau_load(scriptThread, chunkname.c_str(), bytecode.data(), bytecode.size(), 0);
    if (status != LUA_OK) {
        std::string errorMessage{lua_tostring(scriptThread, 1)};
        lua_pop(L, 1);
        return std::unexpected{errorMessage};
    }
    return scriptThread;
}
static auto user_atom(const char* str, size_t len) -> int16_t {
    std::string_view namecall{str, len};
    logger.log("new atom entry {}", namecall);
    constexpr std::array info = compile_time::to_array<Namecall_Atom>();
    auto found = rngs::find_if(info, [&namecall](decltype(info[0])& e) {
        return e.name == namecall;
    });
    if (found == rngs::end(info)) return -1;
    return static_cast<int16_t>(found->value);
}

template <typename Ty>
concept Static_Meta_Pusher = requires (lua_State* L) {
{Ty::push_metatable(L)} -> std::same_as<void>;
};
template <Static_Meta_Pusher Ty, Tag Val = Tag_For<Ty>>
static auto init_tagged(lua_State* L) -> void {
    Ty::push_metatable(L);
    lua_pop(L, 1);
    auto lightuserdataname = std::format("(light){}", compile_time::enum_item<Val>().name);
    lua_setlightuserdataname(L, int(Val), lightuserdataname.c_str());
}
static auto set_up_print_and_warm(lua_State* L, Lou_Console& console) -> void {
    auto print = [](lua_State* L) -> int {
        auto& console = to_tagged<Tag::Lou_Console>(L, lua_upvalueindex(1));
        console.comment(lua::tuple_tostring(L));
        return 0;
    };
    push_tagged(L, console);
    lua_pushcclosure(L, print, "print", 1);
    lua_setglobal(L, "print");
    auto warn = [](lua_State* L) -> int {
        auto& console = to_tagged<Tag::Lou_Console>(L, lua_upvalueindex(1));
        console.warn(lua::tuple_tostring(L));
        return 0;
    };
    push_tagged(L, console);
    lua_pushcclosure(L, warn, "warn", 1);
    lua_setglobal(L, "warn");
}

template <auto Def_X = 0, auto Def_Y = 0, auto Def_Z = 0, auto Def_W = 0>
static auto basic_ctor(lua_State* L) -> int {
    if (lua_isvector(L, 1)) {
        lua_pushvalue(L, 1);
        return 1;
    }
    if (lua_isnone(L, 1)) {
        lua_pushvector(L, Def_X, Def_Y, Def_Z, Def_W);
        return 1;
    }
    return 0;
};
static auto rgba_ctor(lua_State* L) -> int {
    auto value = basic_ctor(L);
    if (value) return value;
    auto [r, g, b, a] = lua::check_args<float, float, float, float>(L);
    lua_pushvector(L, r / 255.f, g / 255.f, b / 255.f, a / 255.f);
    return 1;
}
static auto rgb_ctor(lua_State* L) -> int {
    auto value = basic_ctor<0, 0, 0, 1>(L);
    if (value) return value;
    auto [r, g, b] = lua::check_args<float, float, float>(L);
    lua_pushvector(L, r / 255.f, g / 255.f, b / 255.f, 1);
    return 1;
}
static auto vec4_ctor(lua_State* L) -> int {
    auto basic = basic_ctor(L);
    if (basic) return basic;
    auto [x, y, z, w] = lua::check_args<float, float, float, float>(L);
    lua_pushvector(L, x, y, z, w);
    return 1;
}
static auto vec3_ctor(lua_State* L) -> int {
    auto basic = basic_ctor(L);
    if (basic) return basic;
    auto [x, y, z] = lua::check_args<float, float, float>(L);
    lua_pushvector(L, x, y, z, 0);
    return 1;
}
static auto vec2_ctor(lua_State* L) -> int {
    auto basic = basic_ctor(L);
    if (basic) return basic;
    auto [x, y] = lua::check_args<float, float>(L);
    lua_pushvector(L, x, y, 0, 0);
    return 1;
}
static void register_vector_aliases(lua_State* L) {
    auto color_ctor = [](lua_State* L) -> int {
        return basic_ctor<0, 0, 0, 1>(L);
    };
    const luaL_Reg ctors[] = {
        {"rgb", rgb_ctor},
        {"rgba", rgba_ctor},
        {"vec2", vec2_ctor},
        {"vec3", vec3_ctor},
        {"vec4", vec4_ctor},
        {"rect", vec4_ctor},
        {"line", vec4_ctor},
        {"totuple", [](lua_State* L) -> int {
            auto v = lua::check<lua::Vector_t>(L, 1);
            return lua::values(L, v[0], v[1], v[2], v[3]);
        }},
        {nullptr, nullptr}
    };
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, nullptr, ctors);
    lua_pop(L, 1);
}

auto Lou_State::init_luau() -> void {
    owning.luau.reset(luaL_newstate());
    auto L = lua_state();
    lua_callbacks(L)->useratom = user_atom;
    if (codegen) Luau::CodeGen::create(L);
    luaL_openlibs(L);
    static const luaL_Reg funcs[] = {
        {"loadstring", lua_loadstring},
        {"require", lua_require},
        {"collectgarbage", lua_collectgarbage},
#ifdef CALLGRIND
        {"callgrind", lua_callgrind},
#endif
        {NULL, NULL},
    };
    init_tagged<Lou_Console>(L);
    init_tagged<Lou_State>(L);
    init_tagged<Lou_Keyboard>(L);
    init_tagged<Lou_Mouse>(L);
    init_tagged<Lou_Window>(L);
    init_tagged<Lou_Renderer>(L);
    init_tagged<Lou_Font>(L);
    init_tagged<Lou_Texture>(L);
    init_tagged<Lou_Create_Texture>(L);
    init_tagged<Lou_Callback_Handle>(L);

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, nullptr, funcs);
    lua_pop(L, 1);
    register_vector_aliases(L);
    push_tagged<Lou_State, false>(L, *this);
    lua_setglobal(L, global_name);
    Lou_Font::push_constructor(L);
    lua_setglobal(L, "Font");
    set_up_print_and_warm(L, console);

    luaL_sandbox(L);
}

