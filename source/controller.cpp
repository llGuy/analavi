#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "controller.hpp"
#include "video_viewer.hpp"

lua_State *g_lua_state = NULL;
static const char *cmd_names[(uint32_t)command_type_t::INVALID];

void init_controller() {
    g_lua_state = luaL_newstate();
    luaL_openlibs(g_lua_state);

    cmd_names[(uint32_t)command_type_t::LOAD_FILE] = "load_file";

    BIND_CMD_TO_PROC(command_type_t::LOAD_FILE, cmd_load_file);
}

void submit_cmdstr(const char *cmdstr) {
    int32_t error = luaL_loadbuffer(g_lua_state, cmdstr, strlen(cmdstr), cmdstr) ||
        lua_pcall(g_lua_state, 0, 0, 0);

    if (error) {
        const char *error = lua_tostring(g_lua_state, -1);
        lua_pop(g_lua_state, 1);
    }
}

const char *cmd_type_to_str(command_type_t type) {
    assert((uint32_t)type < (uint32_t)command_type_t::INVALID);
    return cmd_names[(uint32_t)type];
}

template <>
const char *get_from_lua_stack<const char *>(int32_t pos){
    return lua_tostring(g_lua_state, pos);
}

template <>
int32_t get_from_lua_stack<int32_t>(int32_t pos){
    return lua_tonumber(g_lua_state, pos);
}
