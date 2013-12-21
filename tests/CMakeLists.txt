




#ais decoder test
#special case, test decode all messages ... need to perform a special build
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSAD_ENABLE_ALL")
add_executable (${TEST_SAD} ./test_sad.c ../${LIBMM_SRC}/sad.c ../${LIBMM_SRC}/sub0.c)
