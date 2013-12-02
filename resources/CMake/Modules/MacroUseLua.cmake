MACRO(macro_use_lua)

#uv
set( CURRENT_LUA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/lua-5.2.2/src )
set (LUA_A ${CURRENT_LUA_DIR}/liblua.a)
include_directories (${CURRENT_LUA_DIR})


set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -D__WITH_LUA__" )
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -D__WITH_LUA__" )


ENDMACRO(macro_use_lua)




