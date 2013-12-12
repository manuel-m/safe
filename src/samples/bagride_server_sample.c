
#include "uv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bagride.h"
#include "mmtrace.h"


static br_tcp_servers_t tcp_servers = {0};
static br_udp_servers_t udp_servers = {0};
static br_http_servers_t http_servers = {0};

static int on_udp_parse(ssize_t nread_, const uv_buf_t* inbuf_, br_udp_server_t* pserver_) {
    MM_INFO("udp (%d)[%d] %s data (%p)", pserver_->m_port, (int) nread_,
            inbuf_->base,
            pserver_->m_data);
    return 0;
}

static int on_tcp_parse(ssize_t nread_, const uv_buf_t* inbuf_, br_tcp_server_t* pserver_) {
    MM_INFO("tcp (%d)[%d] %s data (%p)", pserver_->m_port, (int) nread_, inbuf_->base,
            pserver_->m_data);
    if (0 > asprintf(&pserver_->m_write_buffer.base, "<<%s>>", inbuf_->base)) return -1;
    pserver_->m_write_buffer.len = strlen(pserver_->m_write_buffer.base);
    return 0;
}

#define RESPONSE1 \
  "HTTP/1.1 200 OK\r\n" \
  "Content-Type: text/plain\r\n" \
  "Content-Length: 13\r\n" \
  "\r\n" \
  "hello1 world\n"

#define RESPONSE2 \
  "HTTP/1.1 200 OK\r\n" \
  "Content-Type: text/plain\r\n" \
  "Content-Length: 13\r\n" \
  "\r\n" \
  "hello2 world\n"

static int gen_hello_response1_cb(br_http_client_t* cli_) {
    cli_->m_resbuf.base = strdup(RESPONSE1);
    cli_->m_resbuf.len = sizeof (RESPONSE1);
    return 0;
}

static int gen_hello_response2_cb(br_http_client_t* cli_) {
    cli_->m_resbuf.base = strdup(RESPONSE2);
    cli_->m_resbuf.len = sizeof (RESPONSE2);
    return 0;
}


int main(void) {


    /* tcp servers  */
    {
        if (0 > br_tcp_servers_init(&tcp_servers, 2)) return -1;
        br_tcp_server_add(&tcp_servers, 6969, on_tcp_parse,2);
        br_tcp_server_add(&tcp_servers, 7070, on_tcp_parse,2);
    }
    
    /* udp servers  */
    {
        if (0 > br_udp_servers_init(&udp_servers, 2)) return -1;
        br_udp_server_add(&udp_servers, 7171, on_udp_parse);
        br_udp_server_add(&udp_servers, 7272, on_udp_parse);
    }    

    /* http servers  */
    {
        if (0 > br_http_servers_init(&http_servers, 2)) return -1;
        br_http_server_add(&http_servers, 7373, gen_hello_response1_cb);
        br_http_server_add(&http_servers, 7474, gen_hello_response2_cb);
    }    
    

    br_run();

    /* cleaning */
    {
        br_tcp_servers_close(&tcp_servers);
        br_udp_servers_close(&udp_servers);
        br_http_servers_close(&http_servers);
    }

    return 0;

}
