#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>
#include <string.h>

#include "mmtrace.h"

struct ais_conf_s {
    int ais_udp_in_port;
    struct {
         int port;
         int max_connections;
         char* name;
         struct {
            char* name_l2;
            int int_l2;
            struct {
              char* name_l3;
              int int_l3;
            } l3; 
         } l2;
    } ais_tcp_server;
};


static int mm_conf_load(struct ais_conf_s* cfg_,const char* f_)
{
    lua_State *L = luaL_newstate();
    luaopen_base(L);
    if (luaL_loadfile(L, f_) || lua_pcall(L, 0, 0, 0)) {
        MM_ERR("cannot run configuration file: %s",f_);
        return 1;
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
        lua_getfield(L, -1, "max_connections");
        if (!lua_isnumber(L, -1)) {
            MM_ERR("ais_tcp_server.max_connections should be a number");
            return -1;
        }
        cfg_->ais_tcp_server.max_connections = (int) lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "name");
        if (!lua_isstring(L, -1)) {
            MM_ERR("ais_tcp_server.name should be a string");
            return -1;
        }
        const char* s = lua_tostring(L, -1);
        cfg_->ais_tcp_server.name = strdup(s);
        lua_pop(L, 1);

        lua_getfield(L, -1, "l2");
        if (!lua_istable(L, -1)) {
          MM_ERR("l2 should be a table, exit");
          return -1;
        }
          lua_getfield(L, -1, "name_l2");
          if (!lua_isstring(L, -1)) {
              MM_ERR("ais_tcp_server.l2.name_l2 should be a string");
              return -1;
          }
          cfg_->ais_tcp_server.l2.name_l2 = strdup(lua_tostring(L, -1));
          lua_pop(L, 1);

          lua_getfield(L, -1, "int_l2");
          if (!lua_isnumber(L, -1)) {
              MM_ERR("ais_tcp_server.l2.int_l2. should be a number");
              return -1;
          }
          cfg_->ais_tcp_server.l2.int_l2 = (int) lua_tonumber(L, -1);
          lua_pop(L,1);

          lua_getfield(L, -1, "l3");
          if (!lua_istable(L, -1)) {
            MM_ERR("l3 should be a table, exit");
            return -1;
          }

          lua_getfield(L, -1, "name_l3");
          if (!lua_isstring(L, -1)) {
              MM_ERR("ais_tcp_server.l2.l3.name_l3 should be a string");
              return -1; 
          }
          cfg_->ais_tcp_server.l2.l3.name_l3 = strdup(lua_tostring(L, -1));
          lua_pop(L, 1); 

          lua_getfield(L, -1, "int_l3");
          if (!lua_isnumber(L, -1)) {
              MM_ERR("ais_tcp_server.l2.l3.int_l3. should be a number");
              return -1; 
          }
          cfg_->ais_tcp_server.l2.l3.int_l3 = (int) lua_tonumber(L, -1);
          lua_pop(L,1);         

          lua_pop(L,1); /* end l3 */
        lua_pop(L,1); /* end l2 */

    }

    lua_close(L);
    return 0;
}

int main(int argc, char** argv){
 
  MM_INFO("start %s", argv[0]);

  static struct ais_conf_s conf;
  if( 2 > argc ){
    MM_ERR("usage: %s /path/to/config",argv[0]);
    return -1;
  }

  if (0 > mm_conf_load(&conf, argv[1])) {
    MM_ERR("Error when loading conf");
    return -1;
  }
  MM_INFO("end %s", argv[0]);
  return 0;
}




