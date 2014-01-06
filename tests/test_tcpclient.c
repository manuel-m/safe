#include <stdio.h>
#include <stdlib.h>

#include "mmtrace.h"
#include "bagride.h"

static int on_read_tcp_client_sample(ssize_t nread_, const br_buf_t* buf_,
        br_tcp_client_t* client_) {
    (void) nread_;
    (void) client_;
    printf("%s", buf_->base);
    return 0;
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    int r = 0;
    
    if(3!=argc){
        MM_ERR("usage: %s addr port", argv[0]);
    }

    br_tcp_client_t client;
    r = br_tcp_client_init(&client, argv[0], argv[1], atoi(argv[2]), on_read_tcp_client_sample);
    if (0 != r){
        return -1;
    }

    br_run();
    return 0;
}