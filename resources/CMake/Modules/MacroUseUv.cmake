MACRO(macro_use_uv)

#uv
set( CURRENT_UV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/libuv )
set (UV_A ${CURRENT_UV_DIR}/.libs/libuv.a)
include_directories (${CURRENT_UV_DIR}/include)


set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -D__WITH_UV__" )
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -D__WITH_UV__" )


ENDMACRO(macro_use_uv)




