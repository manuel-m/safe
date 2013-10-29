MACRO(macro_use_sad)

set( CURRENT_SAD_DIR ${PROJECT_SOURCE_DIR}/../sad)

# static lib path
set (SAD_A ${CURRENT_SAD_DIR}/dist/libsad)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(SAD_A "${SAD_A}_d.a")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

if (CMAKE_BUILD_TYPE MATCHES Release)
  set(SAD_A "${SAD_A}.a")
endif (CMAKE_BUILD_TYPE MATCHES Release)

include_directories (${CURRENT_SAD_DIR}/include)

set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -D__WITH_SAD__" )
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -D__WITH_SAD__" )


ENDMACRO(macro_use_sad)




