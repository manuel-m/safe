MACRO(macro_use_br)

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


#br_net_uv, br_trace
set (BR_NET_UV_A ${CURRENT_BR_DIR}/dist/libbr_net_uv)
set (BR_TRACE_A ${CURRENT_BR_DIR}/dist/libbr_trace)
set (BR_PARSE_A ${CURRENT_BR_DIR}/dist/libbr_parse)


#release / debug 
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(BR_TRACE_A "${BR_TRACE_A}_d.a")
  set(BR_NET_UV_A "${BR_NET_UV_A}_d.a")
  set(BR_PARSE_A "${BR_PARSE_A}_d.a")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

if (CMAKE_BUILD_TYPE MATCHES Release)
  set(BR_TRACE_A "${BR_TRACE_A}.a")
  set(BR_NET_UV_A "${BR_NET_UV_A}.a")
  set(BR_PARSE_A "${BR_PARSE_A}.a")
endif (CMAKE_BUILD_TYPE MATCHES Release)




set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -D__WITH_UV__" )
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -D__WITH_UV__" )


ENDMACRO(macro_use_br)




