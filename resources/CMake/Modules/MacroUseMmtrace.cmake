MACRO(macro_use_mmtrace)

set( CURRENT_MMTRACE_DIR ${PROJECT_SOURCE_DIR}/../mmtrace)

# static lib path
set (MMTRACE_A ${CURRENT_MMTRACE_DIR}/dist/libmmtrace)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(MMTRACE_A "${MMTRACE_A}_d.a")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

if (CMAKE_BUILD_TYPE MATCHES Release)
  set(MMTRACE_A "${MMTRACE_A}.a")
endif (CMAKE_BUILD_TYPE MATCHES Release)

include_directories (${CURRENT_MMTRACE_DIR}/include)

set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -D__WITH_MMTRACE__" )
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -D__WITH_MMTRACE__" )


ENDMACRO(macro_use_mmtrace)




