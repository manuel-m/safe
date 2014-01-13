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

function mmident(level)
    for i=0,level,1 do
      io.write("    ");
    end

end

function mmgen_structnodes(node, nh, level)
    local n
    local v
    for n, v in pairs(node) do
        local nnh = tostring(nh) .. "." .. tostring(n)

        -- table
        if (type(v) == 'table') then    
            -- open {
            mmident(level)
            io.write("struct {\n")
            mmgen_structnodes(v,nnh, level + 1)
            -- close } 
            if(level > 0) then
                mmident(level)
                io.write("} ".. n ..";\n")
            end
        -- not table
        else
            mmident(level)
            io.write (tostring(v) .. " " .. tostring(n) .. ";\n");
        end
  end
end

function mmgen_struct(rootnode, structname)
    io.write("struct " .. tostring(structname) .. "_s {\n")
    mmgen_structnodes(rootnode, structname, 1)
    io.write("};\n")
end

function mmgen_affectnodes(node, nh, level)
    local n
    local v
    for n, v in pairs(node) do
        local nnh = tostring(nh) .. "." .. tostring(n)

        -- table
        if (type(v) == 'table') then    
            -- open one inner level {
            mmident(level - 1)
            io.write("/* " .. tostring(n)  ..  " */\n")
            mmident(level - 1)
            io.write("{\n")           
            mmident(level)
            io.write("lua_getglobal(L,\"" .. tostring(n) .. "\");\n")
            mmident(level)
            io.write("if (!lua_istable(L, -1)) {\n")
            mmident(level + 1)
            io.write("MM_ERR(\"" .. tostring(n) ..  " is not a table\");\n")
            mmident(level + 1)
            io.write("return -1;\n")
            mmident(level)
            io.write("}\n")
             
            mmgen_affectnodes(v,nnh, level + 1)
            -- close one inner level} 
            if(level > 0) then
                mmident(level)
                io.write("lua_pop(L,1);\t/* " .. nnh .. "*/\n")
                mmident(level - 1)
                io.write("}\n")
            end
        -- not table
        else
            mmident(level - 1)
            if(level == 0) then
                io.write("lua_getglobal(L,\"" .. tostring(n) .. "\");\n")
            else
                io.write("lua_getfield(L, -1, \"" .. tostring(n) .."\");\n")
            end
            mmident(level - 1)
            -- numbers
            if(v == 'int' or v == 'double' or v == 'float') then
                io.write("if (!lua_isnumber(L, -1)) {\n")           
                mmident(level)
                io.write("MM_ERR(\"" .. nnh ..  " expected number\");\n")
            end
            -- strings
            if(v == 'char*') then
                io.write("if (!lua_isstring(L, -1)) {\n")           
                mmident(level)
                io.write("MM_ERR(\"" .. nnh ..  " expected string\");\n")
            end
            mmident(level)
            io.write("return -1;\n")
            mmident(level - 1)
            io.write("}\n")

            mmident(level - 1)
            io.write ("cfg_->" .. tostring(nnh) ..  " = ");
            -- numbers
            if(v == 'int' or v == 'double' or v == 'float') then
                io.write("(" .. v .. ")lua_tonumber(L,-1);\n");
            end
            -- strings
            if(v == 'char*') then
                io.write("strdup(lua_tostring(L,-1));\n");
            end

            mmident(level - 1)
            io.write("lua_pop(L,1);\t/* ".. nnh .. " */\n")
        end
  end
end

function mmgen_affect(rootnode, structname)
    -- header
    io.write("static int mm_conf_load(struct " .. tostring(structname) .. "_s* cfg_, const char* f_)\n{\n")
    io.write([[
    lua_State *L = luaL_newstate();
    luaopen_base(L);
    if (luaL_loadfile(L, f_) || lua_pcall(L, 0, 0, 0)) {
        MM_ERR("cannot run configuration file: %s",f_);
        return -1
    }
]])
    mmgen_affectnodes(rootnode, structname, 1)
    -- footer
    io.write("};\n")
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

mmgen_struct(mmapi, mmapi_m.basename)
mmgen_affect(mmapi, mmapi_m.basename)
