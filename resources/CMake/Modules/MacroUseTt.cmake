MACRO(macro_use_tt)

set( CURRENT_TT_DIR ${PROJECT_SOURCE_DIR}/../tt)
include_directories (${CURRENT_TT_DIR}/include)


set (TT_A ${CURRENT_TT_DIR}/dist/libtt)


#release / debug 
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(TT_A "${TT_A}_d.a")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

if (CMAKE_BUILD_TYPE MATCHES Release)
  set(TT_A "${TT_A}.a")
endif (CMAKE_BUILD_TYPE MATCHES Release)


ENDMACRO(macro_use_tt)




