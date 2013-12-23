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
