#include <stdlib.h>
#include <string.h>
#include "sad.h"
#include "br_net.h"
#include "br_trace.h"
#include "br_parse.h"

#define UDP_AIS_PORT 9998

static sad_filter_t filter;

static int on_udp_parse(ssize_t nread_, const uv_buf_t* inbuf_, br_udp_server_t* pserver_) {
    (void) pserver_;
    sad_decode_multiline(&filter, inbuf_->base, nread_);
    return 0;
}

br_udp_server_t udp_servers[] = {
    {
        .m_port = UDP_AIS_PORT,
        .m_user_parse_cb = on_udp_parse
    },
};

static int on_stats_response(br_http_client_t* cli_) {
    cli_->m_resbuf.len = sad_stats_string(&cli_->m_resbuf.base, &filter);

    return 0;
}

br_http_server_t http_servers[] = {
    {
        .m_port = 9997,
        .m_gen_response_cb = on_stats_response
    },
};

int main(void) {

    memset(&filter, 0, sizeof (filter));

    br_udp_register(udp_servers, sizeof (udp_servers) / sizeof (br_udp_server_t));
    br_http_register(http_servers, sizeof (http_servers) / sizeof (br_http_server_t));
    br_run();

    return 0;

}