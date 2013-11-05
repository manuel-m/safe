MACRO(macro_use_mmwincompat)

set( CURRENT_MMWINCOMPAT_DIR ${PROJECT_SOURCE_DIR}/../mmwincompat)

# static lib path
set (MMWINCOMPAT_A ${CURRENT_MMWINCOMPAT_DIR}/dist/libmmwincompat)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(MMWINCOMPAT_A "${MMWINCOMPAT_A}_d.a")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

if (CMAKE_BUILD_TYPE MATCHES Release)
  set(MMWINCOMPAT_A "${MMWINCOMPAT_A}.a")
endif (CMAKE_BUILD_TYPE MATCHES Release)

include_directories (${CURRENT_MMWINCOMPAT_DIR}/include)

set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -D__WITH_MMWINCOMPAT__" )
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -D__WITH_MMWINCOMPAT__" )


ENDMACRO(macro_use_mmwincompat)




