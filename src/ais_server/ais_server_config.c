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
    /* ais_tcp_server */
    {
        lua_getglobal(L,"ais_tcp_server");
        if (!lua_istable(L, -1)) {
            MM_ERR("ais_tcp_server is not a table");
            return -1;
        }
        lua_getfield(L, -1, "name");
        if (!lua_isstring(L, -1)) {
            MM_ERR("ais_tcp_server.name should be a string");
            return -1;
        }
        const char* s = lua_tostring(L, -1);
        cfg_->ais_tcp_server.name = strdup(s);
        lua_pop(L, 1);
        lua_getfield(L, -1, "port");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("ais_tcp_server.port should be a number");
            return -1;
        }
        cfg_->ais_tcp_server.port = (int) lua_tonumber(L, -1);
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
    /* geoserver */
    {
        lua_getglobal(L,"geoserver");
        if (!lua_istable(L, -1)) {
            MM_ERR("geoserver is not a table");
            return -1;
        }
        lua_getfield(L, -1, "y2");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("geoserver.y2 should be a number");
            return -1;
        }
        cfg_->geoserver.y2 = (double) lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "y1");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("geoserver.y1 should be a number");
            return -1;
        }
        cfg_->geoserver.y1 = (double) lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "x1");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("geoserver.x1 should be a number");
            return -1;
        }
        cfg_->geoserver.x1 = (double) lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "x2");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("geoserver.x2 should be a number");
            return -1;
        }
        cfg_->geoserver.x2 = (double) lua_tonumber(L, -1);
        lua_pop(L, 1);

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
    lua_close(L);
    return 0;
}
int ais_server_config_default(const char* f_){(void)f_;return 1;/*TODO*/}
void ais_server_config_close(struct ais_server_config_s* cfg_)
{
(void)cfg_; 
}
