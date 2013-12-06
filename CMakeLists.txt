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

set(MSS_FILTER_DIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
file(MAKE_DIRECTORY ${MSS_FILTER_DIST_DIR})
set(EXECUTABLE_OUTPUT_PATH ${MSS_FILTER_DIST_DIR})
set(LIBRARY_OUTPUT_PATH ${MSS_FILTER_DIST_DIR})

set(MSS_FILTER_EXE mss_filter)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(MSS_FILTER_EXE "${MSS_FILTER_EXE}_d")
endif (CMAKE_BUILD_TYPE MATCHES Debug)


include_directories (src)
add_executable (${MSS_FILTER_EXE} 
src/http_parser.c 
src/mmtrace.c 
src/sub0.c 
src/bagride.c 
src/sad.c
src/mss_filter_config.c
src/mss_filter.c )

target_link_libraries (${MSS_FILTER_EXE} ${UV_A} ${LUA_A} )
target_link_libraries (${MSS_FILTER_EXE} -lpthread -lrt -lm)


if (CMAKE_BUILD_TYPE MATCHES Release)
   install(TARGETS ${MSS_FILTER_EXE} DESTINATION bin)
endif (CMAKE_BUILD_TYPE MATCHES Release)


