MACRO(macro_use_uv)

set( CURRENT_BR_DIR ${PROJECT_SOURCE_DIR}/../bagride)
include_directories (${CURRENT_BR_DIR}/include)




#  N E T W O R K 

#uv
set( CURRENT_UV_DIR ${CURRENT_BR_DIR}/external_libs/libuv )
set (UV_A ${CURRENT_UV_DIR}/.libs/libuv.a)
include_directories (${CURRENT_UV_DIR}/include)

#httpparser
set( CURRENT_HTTPPARSER_DIR ${CURRENT_BR_DIR}/external_libs/libhttp-parser )
set (HTTPPARSER_A ${CURRENT_HTTPPARSER_DIR}/libhttp-parser.a)
include_directories (${CURRENT_HTTPPARSER_DIR})





set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -D__WITH_UV__" )
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -D__WITH_UV__" )


ENDMACRO(macro_use_uv)




