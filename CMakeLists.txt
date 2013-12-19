cmake_minimum_required(VERSION 2.8)

#==============================================================================
# Additional package ressource finders
#==============================================================================
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/resources/CMake/Modules")

#==============================================================================
# The macro below forces the build directory to be different from source directory
#==============================================================================
include(MacroOutOfSourceBuild)
macro_ensure_out_of_source_build("${PROJECT_NAME} requires an out of source build.")

include(MacroLogAll)
macro_logall()

include(MacroDefineCFlags)
macro_define_cflags()

include(MacroUseUv)
macro_use_uv()

include(MacroUseLua)
macro_use_lua()

set(DIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
file(MAKE_DIRECTORY ${DIST_DIR})
set(EXECUTABLE_OUTPUT_PATH ${DIST_DIR})
set(LIBRARY_OUTPUT_PATH ${DIST_DIR})

set(AIS_FILTER ais_filter)
set(AIS_FILTER_EXE ${AIS_FILTER})
set(LIBMM_SRC src/lib)
set(LIBMM_A mm)


if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(AIS_FILTER_EXE "${AIS_FILTER_EXE}_d")
  set(LIBMM_A "${LIBMM_A}_d")
endif (CMAKE_BUILD_TYPE MATCHES Debug)


include_directories (src ${LIBMM_SRC})

add_library(${LIBMM_A} 
            ${LIBMM_SRC}/http_parser.c
            ${LIBMM_SRC}/mmtrace.c 
            ${LIBMM_SRC}/mmpool.c 
            ${LIBMM_SRC}/sub0.c 
            ${LIBMM_SRC}/bagride.c 
            ${LIBMM_SRC}/sad.c)

add_executable (${AIS_FILTER_EXE} src/${AIS_FILTER}/${AIS_FILTER}_config.c src/${AIS_FILTER}/${AIS_FILTER}.c )

target_link_libraries (${AIS_FILTER_EXE} ${LIBMM_A} ${UV_A} ${LUA_A} )
target_link_libraries (${AIS_FILTER_EXE} -lpthread -lrt -lm)


if (CMAKE_BUILD_TYPE MATCHES Release)
   install(TARGETS ${AIS_FILTER_EXE} DESTINATION bin)
endif (CMAKE_BUILD_TYPE MATCHES Release)

