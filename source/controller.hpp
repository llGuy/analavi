#pragma once

#include <utility>
#include <stdint.h>

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

enum class command_type_t : uint32_t {
    LOAD_FILE,
    INVALID
};

void init_controller();

/*
  If the command string is syntactically correct,
  push the command
 */
void submit_cmdstr(const char *cmdstr);
const char *cmd_type_to_str(command_type_t type);

extern struct lua_State *g_lua_state;

// --- Sorry about this stuff ---
template <typename T>
T get_from_lua_stack(int32_t pos);

template <typename ...T, int32_t ...I>
int32_t call_cmd_function_impl(
    int32_t (* proc)(T ...),
    std::integer_sequence<int32_t, I...> stack_indices) {
    constexpr int32_t arg_count = sizeof...(T);
    return proc(get_from_lua_stack<T>(-arg_count + I)...);
}

template <typename ...T>
int32_t call_cmd_function(int32_t (* proc)(T ...)) {
    return call_cmd_function_impl<T...>(
        proc,
        std::make_integer_sequence<int32_t, sizeof...(T)>());
}

#define DECLARE_CMD_PROC(name, ...) \
    int32_t name(__VA_ARGS__); \
    inline int32_t name##_impl(struct lua_State *) { return call_cmd_function(name); }

#define BIND_CMD_TO_PROC(type, func)                    \
    lua_pushcfunction(g_lua_state, func##_impl);        \
    lua_setglobal(g_lua_state, cmd_type_to_str(type))
