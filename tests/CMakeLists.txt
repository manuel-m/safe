#tests
set(TEST_POOL test_pool)
set(TEST_SAD test_sad)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(TEST_POOL "${TEST_POOL}_d")
  set(TEST_SAD "${TEST_SAD}_d")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

#pool 
add_executable (${TEST_POOL} ./test_pool.c ../${LIBMM_SRC}/mmpool.c)

#ais decoder test
#special case, test decode all messages ... need to perform a special build
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSAD_ENABLE_ALL")
add_executable (${TEST_SAD} ./test_sad.c ../${LIBMM_SRC}/sad.c ../${LIBMM_SRC}/sub0.c)
