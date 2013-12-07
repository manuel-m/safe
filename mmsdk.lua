#!./libs/lua-5.2.2/src/lua
-- mm sdk tools

-- [mmerr] --------------------------------------------------------------------
mmerrs={ 
  missing_config="missing configuration file",
  missing_argument="missing argument",
  missing_api_m="missing api meta",
  missing_api_m_dirname="missing api meta dirname",
  missing_api_m_basename="missing api meta basename",
  missing_api_m_version="missing api meta version",
  missing_api_m_define="missing api meta define",
  missing_api="missing api",
}
mmerr = function (str) 
  print(str)
  os.exit(1)
end
-- [mmgen_api_c] -------------------------------------------------------------------
function mmgen_api_c()
  
  local filename_c = mmapi_m.dirname .. "/" .. mmapi_m.basename ..".c";
  local f_c = io.open (filename_c,"w")
  local api_t = "struct " .. mmapi_m.basename .. "_s";  

-- c header
  f_c:write([[#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "mmtrace.h"

]])
  f_c:write("#include \"" .. mmapi_m.basename .. ".h\" \n")
  
-- c load config function  
  f_c:write("int " .. mmapi_m.basename .. "_load(" .. api_t .. "* cfg_,const char* f_)\n{\n")
  f_c:write([[
    lua_State *L = luaL_newstate();
    luaopen_base(L);
    if (luaL_loadfile(L, f_) || lua_pcall(L, 0, 0, 0)) {
        MM_ERR("cannot run configuration file: %s",f_);
        return 1;
    }
]])
-- iter on params
    for n,v in pairs(mmapi) do
      
        f_c:write("    /* " .. n .. " */\n    {\n")
        f_c:write("        lua_getglobal(L,\"" .. n .. "\");\n");          
          
        if (type(v) == 'table') then
          
                f_c:write("        if (!lua_istable(L, -1)) {\n")
                f_c:write("            MM_ERR(\"" .. n .. " is not a table\");\n");
                f_c:write("            return 1;\n")
                f_c:write("        }\n")
                
                -- iter on all table keys
                for n2,v2 in pairs(v) do
                    f_c:write("        lua_getfield(L, -1, \"" ..  n2 .. "\");\n")
                    
                    -- numbers
                    if(v2 == 'int' or v2 == 'double' or v2 == 'float') then
                        f_c:write("        if (!lua_isnumber(L, -1)) {\n")
                        f_c:write("            MM_ERR(\""..n .."." .. n2 .. " should be a number\");\n")
                        f_c:write("            return 1;\n")
                        f_c:write("        }\n")                   
                        f_c:write("        cfg_->" .. n .. "." .. n2 .." = (".. v2 .. ") lua_tonumber(L, -1);\n")
                        f_c:write("        lua_pop(L, 1);\n")
                    end
              end        
                
        -- not table  
        else  
              
            -- numbers  
            if(v == 'int' or v == 'double' or v == 'float') then

                f_c:write("        if (!lua_isnumber(L, -1)) {\n");
                f_c:write("            MM_ERR(\"" .. n .. " should be a number\");\n");
                f_c:write("            return 1;\n")
                f_c:write("        }\n")        
                f_c:write("        cfg_->" .. n .. " = (" .. v .. ") lua_tonumber(L, -1);\n")
                f_c:write("        lua_pop(L, 1);\n")
                f_c:write("        MM_INFO(\"" .. n .. "=%d\", cfg_->".. n .. ");")      
            end
        end
        f_c:write("\n    }\n")        
    end

  f_c:write([[
    return 0;
}
]])
  
  
  f_c:write("int " .. mmapi_m.basename .. "_default(const char* f_){(void)f_;return 1;}\n")
      
  f_c:close()
  
  print("[INFO] " .. filename_c .. " generated")
   
end

-- [mmgen_api_h] -------------------------------------------------------------------
function mmgen_api_h()
  
-- struct
  local api_t = "struct " .. mmapi_m.basename .. "_s";
  
-- open files
  local filename_h = mmapi_m.dirname .. "/" .. mmapi_m.basename ..".h";
  local f_h = io.open (filename_h,"w")

-- h header  
  f_h:write("#ifndef " .. mmapi_m.define .. "\n")
  f_h:write("#define " .. mmapi_m.define .. "\n")
  f_h:write([[
    
#ifdef  __cplusplus
extern "C" {
#endif

]])

-- h config struct definition
  f_h:write(api_t .. "{\n" );

  for n,v in pairs(mmapi) do
    if (type(v) == 'table') then
      f_h:write(" struct {\n");
      for n2,v2 in pairs(v) do
        f_h:write("     " .. v2 .. " " .. n2 ..";\n")
      end
      f_h:write(" } " .. n .. ";\n");
    else  
      f_h:write(" " .. v .. " " .. n ..";\n")
    end
  end
  f_h:write("};\n" );

  f_h:write("int " .. mmapi_m.basename .. "_load(" .. api_t .. "*,const char*);\n")
  f_h:write("int " .. mmapi_m.basename .. "_default(const char* f_);\n")
  
-- h footer      
  f_h:write([[      
    
#ifdef  __cplusplus
}
#endif

]])
  f_h:write("#endif /* " .. mmapi_m.define .." */ \n")      


-- close files
  f_h:close()
  
  print("[INFO] " .. filename_h .. " generated")
   
end



-- [ main ] --------------------------------------------------------------------
if not arg[1] then mmerr(mmerrs.missing_argument) end
local configuration_file = arg[1];
print("[INFO] loading:" .. arg[1])

api_file = assert(loadfile(configuration_file))
api_file()

-- check api name:mmapi_name
if not mmapi_m then mmerr(mmerrs.missing_api_m) end
if not mmapi_m.basename then mmerr(mmerrs.missing_api_basename) end
if not mmapi_m.dirname then mmerr(mmerrs.missing_api_dirname) end
if not mmapi_m.version then mmerr(mmerrs.missing_api_version) end
if not mmapi_m.define then mmerr(mmerrs.missing_api_define) end
if not mmapi then mmerr(mmerrs.missing_api) end


mmgen_api_h()
mmgen_api_c()






    