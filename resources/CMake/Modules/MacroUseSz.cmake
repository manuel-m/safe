MACRO(macro_use_sz)

set( CURRENT_SZ_DIR ${PROJECT_SOURCE_DIR}/../sub0)
include_directories (${CURRENT_SZ_DIR}/include)


set (SZ_A ${CURRENT_SZ_DIR}/dist/libsz)


#release / debug 
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(SZ_A "${SZ_A}_d.a")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

if (CMAKE_BUILD_TYPE MATCHES Release)
  set(SZ_A "${SZ_A}.a")
endif (CMAKE_BUILD_TYPE MATCHES Release)


ENDMACRO(macro_use_sz)




