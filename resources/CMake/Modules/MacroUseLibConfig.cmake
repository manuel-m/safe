MACRO(macro_use_libconfig)

#libconfig
set ( CURRENT_LIBCONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/libconfig-1.4.9/lib )
set ( LIBCONFIG_A ${CURRENT_LIBCONFIG_DIR}/.libs/libconfig.a )
include_directories (${CURRENT_LIBCONFIG_DIR})

set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -D__WITH_LIBCONFIG__" )
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -D__WITH_LIBCONFIG__" )


ENDMACRO(macro_use_libconfig)




