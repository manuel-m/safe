#tests
set(TEST_POOL test_pool)
set(TEST_CB test_cb)
set(TEST_SAD test_sad)
set(TEST_TCPCLIENT test_tcpclient)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(TEST_CB "${TEST_CB}_d")
  set(TEST_POOL "${TEST_POOL}_d")
  set(TEST_SAD "${TEST_SAD}_d")
  set(TEST_TCPCLIENT "${TEST_TCPCLIENT}_d")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

#circular buffer
add_executable (${TEST_CB} ./test_cb.c ../${LIBMM_SRC}/mmcb.c)

#pool 
add_executable (${TEST_POOL} ./test_pool.c ../${LIBMM_SRC}/mmpool.c)

#tcpclient
add_executable (${TEST_TCPCLIENT} ./test_tcpclient.c ../${LIBMM_SRC}/bagride.c)
target_link_libraries (${TEST_TCPCLIENT} ${LIBMM_A} ${UV_A} )
target_link_libraries (${TEST_TCPCLIENT} -lpthread -lrt -lm)

#ais decoder test
#special case, test decode all messages ... need to perform a special build
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSAD_ENABLE_ALL")
add_executable (${TEST_SAD} ./test_sad.c ../${LIBMM_SRC}/sad.c ../${LIBMM_SRC}/sub0.c)
