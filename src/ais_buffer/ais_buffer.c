#include <stdlib.h>
#include <string.h>

#include "sad.h"
#include "bagride.h"
#include "mmtrace.h"
#include "mmconfig.h"
#include "mmvector.h"
#include "netchannel.h"
#include "mmdefs.h"

#define HELP_USAGE "usage: ais_buffer cfg_file" 

#include <stdio.h>
#include <stdlib.h>

#include "mmtrace.h"
#include "bagride.h"
#include "mmconfig.h"

static struct {
    int buffer_max;
    int io_admin_http_port;
    channel_in_values_t in_ais;
    channel_out_values_t out_ais_tcp_server;
    channel_out_values_t out_ais_tcp_error;
} values;

//static struct {
//    
//    
//} ais_buffer;

static br_http_server_t srv_out_http_stats;
static br_tcp_client_t in_ais_tcpclient;

static int on_stats_response(br_http_client_t* cli_) {
    cli_->m_resbuf.base = strdup("TODO");
    cli_->m_resbuf.len = strlen(cli_->m_resbuf.base);
    return 0;
}

static int load_config(config_t* cfg_) {
    int r = 0;

    MM_CFG_GET_INT(cfg_, buffer_max, values);
    MM_CFG_GET_STR(cfg_, in_ais.name, values);
    MM_CFG_GET_STR(cfg_, in_ais.addr, values);
    MM_CFG_GET_INT(cfg_, in_ais.port, values);

    /* only tcp client */
    values.in_ais.type_id = NC_IN_TCPCLI;

    MM_CFG_GET_INT(cfg_, io_admin_http_port, values);

    MM_CFG_GET_STR(cfg_, out_ais_tcp_server.name, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_server.port, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_server.max_connections, values);

    MM_CFG_GET_STR(cfg_, out_ais_tcp_error.name, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_error.port, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_error.max_connections, values);

end:
    return r;

err:
    r = -1;
    goto end;
}

static int on_read_tcp_client_in_ais(ssize_t nread_, const br_buf_t* buf_,
        br_tcp_client_t* client_) {
    (void) nread_;
    (void) client_;
    MM_INFO("%s", buf_->base);
    return 0;
}

int main(int argc, char **argv) {
    int r = 0;
    if (2 > argc) MM_GERR(HELP_USAGE);

    config_t cfg;
    MM_CONFIG_INIT(&cfg, argv[1]);

    MM_INFO("version=\"%s\"", MM_VERSION_INFO);
    MM_INFO("conf=\"%s\"", argv[1]);

    /* http server  */
    br_http_server_init(&srv_out_http_stats, values.io_admin_http_port,
            on_stats_response);


    r = br_tcp_client_init(&in_ais_tcpclient, values.in_ais.name,
            values.in_ais.addr, values.in_ais.port, on_read_tcp_client_in_ais);
    if (0 != r) {
        return -1;
    }
    br_run();

end:

    //    br_tcp_server_close(&srv_out_tcp_filtered_ais);
    //    br_tcp_server_close(&srv_out_tcp_filtered_ais_error);
    config_destroy(&cfg);
    return r;

err:
    r = -1;
    goto end;

}