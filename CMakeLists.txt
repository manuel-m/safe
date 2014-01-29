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

include(MacroUseLibConfig)
macro_use_libconfig()

set(DIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
file(MAKE_DIRECTORY ${DIST_DIR})
set(EXECUTABLE_OUTPUT_PATH ${DIST_DIR})
set(LIBRARY_OUTPUT_PATH ${DIST_DIR})

set(AIS_DECODER ais_decoder)
set(AIS_SIMPLE_UDP_REPLAYER ais_simple_udp_replayer)
set(AIS_HISTORY_FILTER ais_history_filter)
set(AIS_FILTER ais_filter)
set(AIS_BUFFER ais_buffer)
set(AIS_CONFIG ais_config)
set(AIS_SERVER ais_server)
set(AIS_DECODER_EXE ${AIS_DECODER})
set(AIS_FILTER_EXE ${AIS_FILTER})
set(AIS_HISTORY_FILTER_EXE ${AIS_HISTORY_FILTER})
set(AIS_SIMPLE_UDP_REPLAYER_EXE ${AIS_SIMPLE_UDP_REPLAYER})
set(AIS_BUFFER_EXE ${AIS_BUFFER})
set(AIS_SERVER_EXE ${AIS_SERVER})
set(AIS_CONFIG_EXE ${AIS_CONFIG})

set(LIBMM_SRC src/lib)
set(LIBMM_A mm)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(AIS_DECODER_EXE "${AIS_DECODER_EXE}_d")
  set(AIS_FILTER_EXE "${AIS_FILTER_EXE}_d")
  set(AIS_HISTORY_FILTER_EXE "${AIS_HISTORY_FILTER_EXE}_d")
  set(AIS_SIMPLE_UDP_REPLAYER_EXE "${AIS_SIMPLE_UDP_REPLAYER}_d")
  set(AIS_BUFFER_EXE "${AIS_BUFFER_EXE}_d")
  set(AIS_SERVER_EXE "${AIS_SERVER_EXE}_d")
  set(AIS_CONFIG_EXE "${AIS_CONFIG_EXE}_d")
  set(LIBMM_A "${LIBMM_A}_d")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

include_directories (src ${LIBMM_SRC})

add_library(${LIBMM_A} 
            ${LIBMM_SRC}/http_parser.c
            ${LIBMM_SRC}/mmtrace.c 
            ${LIBMM_SRC}/mmcb.c 
            ${LIBMM_SRC}/mmpool.c 
            ${LIBMM_SRC}/sub0.c 
            ${LIBMM_SRC}/netchannel.c 
            ${LIBMM_SRC}/bagride.c 
            ${LIBMM_SRC}/sad.c)
            
#ais decoder
# only decode 1,2,3 type messages
add_executable (${AIS_DECODER_EXE} src/${AIS_DECODER}.c  ${LIBMM_SRC}/sad.c ${LIBMM_SRC}/sub0.c)

# only decode 1,2,3 type messages + meditarrean hardcoded filter
add_executable (${AIS_HISTORY_FILTER_EXE} src/${AIS_HISTORY_FILTER}/${AIS_HISTORY_FILTER}.c  ${LIBMM_SRC}/sad.c ${LIBMM_SRC}/sub0.c)

# only replay 1,2,3 type messages 
add_executable (${AIS_SIMPLE_UDP_REPLAYER_EXE} src/${AIS_SIMPLE_UDP_REPLAYER}/${AIS_SIMPLE_UDP_REPLAYER}.c  ${LIBMM_SRC}/sad.c ${LIBMM_SRC}/sub0.c ${LIBMM_SRC}/mmtrace.c )


#ais config
add_executable (${AIS_CONFIG_EXE}  src/${AIS_CONFIG}/${AIS_CONFIG}.c )
target_link_libraries (${AIS_CONFIG_EXE} ${LIBMM_A} ${UV_A} ${LUA_A} ${LIBCONFIG_A})
target_link_libraries (${AIS_CONFIG_EXE} -lpthread -lrt -lm)


#ais filter            
add_executable (${AIS_FILTER_EXE} src/${AIS_FILTER}/${AIS_FILTER}.c )
target_link_libraries (${AIS_FILTER_EXE} ${LIBMM_A} ${UV_A} ${LIBCONFIG_A} )
target_link_libraries (${AIS_FILTER_EXE} -lpthread -lrt -lm)

#ais buffer            
add_executable (${AIS_BUFFER_EXE} src/${AIS_BUFFER}/${AIS_BUFFER}.c )
target_link_libraries (${AIS_BUFFER_EXE} ${LIBMM_A} ${UV_A} ${LIBCONFIG_A} )
target_link_libraries (${AIS_BUFFER_EXE} -lpthread -lrt -lm)

#ais server            
#add_executable (${AIS_SERVER_EXE} src/${AIS_SERVER}/${AIS_SERVER}_config.c src/${AIS_SERVER}/${AIS_SERVER}.c )
#target_link_libraries (${AIS_SERVER_EXE} ${LIBMM_A} ${UV_A} ${LUA_A} )
#target_link_libraries (${AIS_SERVER_EXE} -lpthread -lrt -lm)



if (CMAKE_BUILD_TYPE MATCHES Release)
   install(TARGETS ${AIS_FILTER_EXE} DESTINATION bin)
#   install(TARGETS ${AIS_SERVER_EXE} DESTINATION bin)
endif (CMAKE_BUILD_TYPE MATCHES Release)

add_subdirectory(coverage)

# tests enable full ais message decoding
add_subdirectory(tests)




