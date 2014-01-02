#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>
#include <string.h>

#include "mmtrace.h"

#include "ais_server_config.h" 
int ais_server_config_load(struct ais_server_config_s* cfg_,const char* f_)
{
    lua_State *L = luaL_newstate();
    luaopen_base(L);
    if (luaL_loadfile(L, f_) || lua_pcall(L, 0, 0, 0)) {
        MM_ERR("cannot run configuration file: %s",f_);
        return 1;
    }
    /* max_ships */
    {
        lua_getglobal(L,"max_ships");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("max_ships should be a number");
            return -1;
        }
        cfg_->max_ships = (int) lua_tonumber(L, -1);
        lua_pop(L, 1);
        MM_INFO("max_ships=%d", cfg_->max_ships);
    }
    /* step_ships */
    {
        lua_getglobal(L,"step_ships");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("step_ships should be a number");
            return -1;
        }
        cfg_->step_ships = (int) lua_tonumber(L, -1);
        lua_pop(L, 1);
        MM_INFO("step_ships=%d", cfg_->step_ships);
    }
    /* ais_udp_in_port */
    {
        lua_getglobal(L,"ais_udp_in_port");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("ais_udp_in_port should be a number");
            return -1;
        }
        cfg_->ais_udp_in_port = (int) lua_tonumber(L, -1);
        lua_pop(L, 1);
        MM_INFO("ais_udp_in_port=%d", cfg_->ais_udp_in_port);
    }
    /* ais_tcp_server */
    {
        lua_getglobal(L,"ais_tcp_server");
        if (!lua_istable(L, -1)) {
            MM_ERR("ais_tcp_server is not a table");
            return -1;
        }
        lua_getfield(L, -1, "port");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("ais_tcp_server.port should be a number");
            return -1;
        }
        cfg_->ais_tcp_server.port = (int) lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "name");
        if (!lua_isstring(L, -1)) {
            MM_ERR("ais_tcp_server.name should be a string");
            return -1;
        }
        const char* s = lua_tostring(L, -1);
        cfg_->ais_tcp_server.name = strdup(s);
        lua_pop(L, 1);
        lua_getfield(L, -1, "max_connections");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("ais_tcp_server.max_connections should be a number");
            return -1;
        }
        cfg_->ais_tcp_server.max_connections = (int) lua_tonumber(L, -1);
        lua_pop(L, 1);

    }
    /* admin_http_port */
    {
        lua_getglobal(L,"admin_http_port");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("admin_http_port should be a number");
            return -1;
        }
        cfg_->admin_http_port = (int) lua_tonumber(L, -1);
        lua_pop(L, 1);
        MM_INFO("admin_http_port=%d", cfg_->admin_http_port);
    }
    /* min_ships */
    {
        lua_getglobal(L,"min_ships");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("min_ships should be a number");
            return -1;
        }
        cfg_->min_ships = (int) lua_tonumber(L, -1);
        lua_pop(L, 1);
        MM_INFO("min_ships=%d", cfg_->min_ships);
    }
    lua_close(L);
    return 0;
}
int ais_server_config_default(const char* f_){(void)f_;return 1;/*TODO*/}
void ais_server_config_close(struct ais_server_config_s* cfg_)
{
(void)cfg_; 
}
