#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "bagride.h"
#include "sub0.h"
#include "mmtrace.h"

static int br_isipv4(const char *ip_, size_t size_) {
    int i;
    long int j;
    const char *start;
    char *end;

    for (i = 0, start = ip_; i < 4; i++, start = end + 1) {
        if (!isdigit((int) (unsigned char) *start)) return 0;
        errno = 0;
        j = strtol(start, &end, 10);
        if (j > 255) return 0;
        if (errno != 0 || j < 0 || j > 255) return 0;
    }
    if (i < 4 || end != ip_ + size_) return 0;
    return 1;
}

void br_tcp_servers_close(mmpool_t* srv_pool_) {

    mmpool_iter_t iter = {
        .m_pool = srv_pool_,
        .m_index = 0
    };
    
    br_tcp_server_t* srv = mmpool_iter_next(&iter);

    while (srv) {
        mmpool_free(srv->m_clients);
        srv = mmpool_iter_next(&iter);
    }
    mmpool_free(srv_pool_);
}

static void on_alloc_buffer(uv_handle_t *handle_, size_t suggested_size_,
        uv_buf_t* buf_) {
    (void) handle_;
    buf_->base = (char*) calloc(suggested_size_, sizeof (char));
    buf_->len = suggested_size_;
}

static void on_close(uv_handle_t* client_handle_) {

    mmpool_item_t* client_pool_item = (mmpool_item_t*) client_handle_->data;
    br_tcp_server_t* server = (br_tcp_server_t*) client_pool_item->m_parent->m_userdata;

    mmpool_giveback(client_pool_item);

    MM_INFO("%s:%d disconnect (%d/%d) (%p)",
            server->m_name,
            server->m_port,
            server->m_clients->m_taken_len,
            server->m_clients->m_alloc_max,
            client_handle_);
}

static void on_close_quick(uv_handle_t* client_handle_) {
    free(client_handle_);
}

static void on_after_write(uv_write_t* req_, int status_) {
    MM_ASSERT(status_ >= 0);
    char *base = (char*) req_->data;
    free(base);
    free(req_);
}

static void on_tcp_read(uv_stream_t* stream_, ssize_t nread_, const uv_buf_t* read_buf_) {
    if (nread_ > 0) {
        MM_INFO("on_tcp_read (%p)", stream_);

        mmpool_item_t* client_pool_item = (mmpool_item_t*) stream_->data;
        br_tcp_server_t* tcp_server = (br_tcp_server_t*) client_pool_item->m_parent->m_userdata;
        br_buf_t* write_buffer = &tcp_server->m_write_buffer;
        br_tcp_server_parser_cb user_parse_cb = (br_tcp_server_parser_cb) tcp_server->m_user_parse_cb;
        MM_ASSERT(0 == user_parse_cb(nread_, read_buf_, tcp_server));
        if (write_buffer->len) {
            uv_write_t *req = (uv_write_t *) calloc(1, sizeof (uv_write_t));
            req->data = (void*) write_buffer->base;
            uv_write(req, stream_, (uv_buf_t*) write_buffer, 1, on_after_write);
        }
    } else {
        uv_close((uv_handle_t*) stream_, on_close);
    }
    free(read_buf_->base);
}

int br_tcp_write_string(br_tcp_server_t* server_, const char* str_, size_t len_) {

    mmpool_iter_t iter = {
        .m_index = 0,
        .m_pool = server_->m_clients
    };

    uv_stream_t* pclient = (uv_stream_t*) mmpool_iter_next(&iter);

    while (pclient) {
        br_buf_t* write_buffer = &server_->m_write_buffer;
        write_buffer->len = len_;
        write_buffer->base = strdup(str_);
        uv_write_t *req = (uv_write_t *) calloc(1, sizeof (uv_write_t));
        req->data = (void*) write_buffer->base;
        uv_write(req, pclient, (uv_buf_t*) write_buffer, 1, on_after_write);
        pclient = (uv_stream_t*) mmpool_iter_next(&iter);
    }
    return 0;
}

static void on_udp_recv(uv_udp_t* stream_, ssize_t nread_, const uv_buf_t* read_buff_,
        const struct sockaddr* addr_, unsigned flags_) {

    (void) addr_;
    (void) flags_;

    if (nread_ > 0) {
        br_udp_server_t* server_udp = (br_udp_server_t*) stream_->data;
        br_udp_server_parser_cb user_parse_cb = (br_udp_server_parser_cb) server_udp->m_user_parse_cb;
        MM_ASSERT(0 == user_parse_cb(nread_, read_buff_, server_udp));
    }
    free(read_buff_->base);
}

static void on_udp_send(uv_udp_send_t* req_, int status) {
    (void) status;
    char *base = (char*) req_->data;
    free(base);
    free(req_);
}

static void server_on_connect(uv_stream_t* server_handle_, int status_) {

    MM_ASSERT(status_ >= 0);
    br_tcp_server_t* server = (br_tcp_server_t*) server_handle_->data;
    mmpool_item_t* client_pool_item = mmpool_take(server->m_clients);

    uv_tcp_t *pclient;

    if (NULL == client_pool_item) {
        MM_WARN("%s:%d max connections reached (%d) ... system wont accept new connections",
                server->m_name,
                server->m_port,
                server->m_clients->m_alloc_max);
        /* workaround : connection is accepted and immediatly closed.
         This allow to re-accept new connections after some disconnections ...*/
        pclient = malloc(sizeof (uv_tcp_t));
        uv_tcp_init(uv_default_loop(), pclient);
        uv_accept(server_handle_, (uv_stream_t*) pclient);
        uv_close((uv_handle_t*) pclient, on_close_quick);
        return;

    }
    pclient = (uv_tcp_t *) client_pool_item->m_p;

    uv_tcp_init(uv_default_loop(), pclient);
    pclient->data = client_pool_item;
    
    int r = uv_accept(server_handle_, (uv_stream_t*) pclient);

    if (0 == r) {
        MM_INFO("%s:%d connect (%d/%d) (serv:%p cli:%p)",
                server->m_name,
                server->m_port,
                server->m_clients->m_taken_len,
                server->m_clients->m_alloc_max,
                server_handle_, pclient);
        uv_read_start((uv_stream_t*) pclient, on_alloc_buffer, on_tcp_read);
    } else {
        MM_ERR("connection failed:%s", uv_strerror(r));
        uv_close((uv_handle_t*) pclient, on_close);
    }
}

int br_udp_server_add(mmpool_t* srv_pool_, int port_, void* user_parse_cb_) {

    mmpool_item_t* pool_item = mmpool_take(srv_pool_);
    int r;
    if (NULL == pool_item) return -1;
    br_udp_server_t* server = (br_udp_server_t*) pool_item->m_p;
    server->m_port = port_;
    server->m_user_parse_cb = user_parse_cb_;
    MM_INFO("udp in %d", server->m_port);
    
    r = uv_udp_init(uv_default_loop(), &server->m_handler);
    if(0 != r){
      MM_ERR("udp server init failed: %s", uv_strerror(r));
      return -1;
    }
    server->m_handler.data = server;
    MM_ASSERT(0 == uv_ip4_addr("0.0.0.0", server->m_port, &server->m_socketaddr));
    MM_ASSERT(0 == uv_udp_bind(&server->m_handler, (const struct sockaddr*) &server->m_socketaddr, 0));
    MM_ASSERT(0 == uv_udp_recv_start(&server->m_handler, on_alloc_buffer, on_udp_recv));
    return 0;
}

int br_tcp_server_add(mmpool_t* srv_pool_, const char* name_, int port_,
        void* user_parse_cb_, int max_connections_) {
    mmpool_item_t* pool_item = mmpool_take(srv_pool_);
    if (NULL == pool_item) return -1;
    br_tcp_server_t* server = (br_tcp_server_t*) pool_item->m_p;
    server->m_name = name_;
    server->m_port = port_;
    server->m_user_parse_cb = user_parse_cb_;
    MM_INFO("%s:%d server listening", server->m_name, server->m_port);
    uv_tcp_init(uv_default_loop(), &server->m_server_handler);
    server->m_server_handler.data = server;

    /* clients */
    server->m_clients = mmpool_easy_new(max_connections_,sizeof (br_tcp_t), server);

    MM_ASSERT(0 == uv_ip4_addr("0.0.0.0", server->m_port, &server->m_socketaddr));
    MM_ASSERT(0 == uv_tcp_bind(&server->m_server_handler, (const struct sockaddr*) &server->m_socketaddr));
    MM_ASSERT(0 == uv_listen((uv_stream_t*) & server->m_server_handler, max_connections_,
            server_on_connect));
    return 0;
}

static void on_resolved_udp_client(uv_getaddrinfo_t *resolver_, int status_,
        struct addrinfo *res_) {

    mmpool_item_t* cli_pool_item = (mmpool_item_t*) resolver_->data;
    br_udp_client_t* cli = (br_udp_client_t*) cli_pool_item->m_p;

    if (0 > status_) {
        MM_ERR("dns failed for:%s %s", cli->m_addr,uv_strerror(status_));
        mmpool_giveback(cli_pool_item);
        return;
    }
    else
    {
        char ip_addr[17] = {'\0'};
        uv_ip4_name((struct sockaddr_in*) res_->ai_addr, ip_addr, 16);
        MM_INFO("%s.ip=%s", cli->m_addr, ip_addr);
        strcpy(cli->m_addr, ip_addr);
        int r = uv_ip4_addr(cli->m_addr, cli->m_port, &cli->m_socketaddr);
        if(0 != r) MM_ERR("bad resolving:%s, %s", uv_strerror(r));
    }

    free(resolver_);
    uv_freeaddrinfo(res_);
}

int br_udp_client_register(mmpool_item_t* cli_pool_item_) {
    
    br_udp_client_t* cli = (br_udp_client_t*) cli_pool_item_->m_p;
    int r = uv_udp_init(uv_default_loop(), &cli->m_handler);
    if (0 != r) {
        MM_ERR("new udp client failed :%s",uv_strerror(r));
            return -1;
    }    

    /* invalid ip, so we try a DNS resolution */
    if (0 == br_isipv4(cli->m_addr, strlen(cli->m_addr))) {
        uv_getaddrinfo_t* resolver = calloc(1, sizeof (uv_getaddrinfo_t));
        struct addrinfo hints;
        hints.ai_family = PF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = 0;
        resolver->data = cli_pool_item_;

        /* DNS resolution error */
        r = uv_getaddrinfo(uv_default_loop(), resolver,
                on_resolved_udp_client, cli->m_addr, NULL, &hints);
        
        if (0 != r) {
            MM_ERR("dns resolution failed for:%s, %s", cli->m_addr,uv_strerror(r));
            return -1;
        }

    } else {
        r = uv_ip4_addr(cli->m_addr, cli->m_port, &cli->m_socketaddr);
        if (0 != r) {
           MM_ERR("invalid address:%s, %s", cli->m_addr,uv_strerror(r));
           return -1;
       }
    }
    return 0;
}

int br_udp_client_add(mmpool_t* cli_pool_, const char* target_) {

    mmpool_item_t* pool_item = mmpool_take(cli_pool_);
    if (NULL == pool_item) return -1;
    br_udp_client_t* cli = (br_udp_client_t*) pool_item->m_p;

    sub0_line_t line;
    sub0_substring_t* sub = NULL;
    sub0_line_prepare(target_, strlen(target_), ':', &line);

    /* 1st field:addr */
    sub = sub0_line_next_substring(&line);
    if (NULL == sub || 0 == sub->n || NULL == sub->start) return -1;
    if (BR_MAX_ADDR_SIZE < sub->n) return -1;

    memcpy(cli->m_addr, sub->start, sub->n);
    cli->m_addr[sub->n] = '\0';

    /* 2nd field:port */
    sub = sub0_line_next_substring(&line);
    if (NULL == sub || 0 == sub->n || NULL == sub->start) return -1;
    cli->m_port = atoi(sub->start);
    if (0 > br_udp_client_register(pool_item)) return -1;
    return 0;
}

void br_udp_client_send(br_udp_client_t* cli_, const char* str_) {
    uv_buf_t udp_sentence;
    uv_udp_send_t* send_req = (uv_udp_send_t*) calloc(1, sizeof (uv_udp_send_t));
    udp_sentence.base = (char*) strdup(str_);
    udp_sentence.len = strlen(str_) + 1;
    send_req->data = udp_sentence.base; /* no memory leak */

    int r = uv_udp_send(send_req, &cli_->m_handler, &udp_sentence, 1,
            (const struct sockaddr *) &cli_->m_socketaddr, on_udp_send);

    if (0 != r) {
        MM_ERR("udp send failed for:%s", cli_->m_addr, uv_strerror(r));
    }
}

void br_udp_clients_send(mmpool_t* cli_pool_, const char* str_) {

    mmpool_iter_t iter = {
        .m_index = 0,
        .m_pool = cli_pool_
    };

    br_udp_client_t* pclient = (br_udp_client_t*) mmpool_iter_next(&iter);

    while (pclient) {
        br_udp_client_send(pclient, str_);
        pclient = (br_udp_client_t*) mmpool_iter_next(&iter);
    }
}

/**
 * http
 */
static void on_http_close(uv_handle_t* handle_) {
    br_http_client_t* cli = (br_http_client_t*) handle_->data;
    MM_INFO("(%5d) connection closed ", cli->m_request_num);
    free(cli->m_resbuf.base);
    free(cli);
}

static void on_http_after_write(uv_write_t* req_, int status_) {
    MM_ASSERT(status_ >= 0);
    uv_close((uv_handle_t*) req_->handle, on_http_close);
}

static int on_headers_complete(http_parser* parser) {
    br_http_client_t* cli = (br_http_client_t*) parser->data;
    MM_INFO("(%5d) http message parsed", cli->m_request_num);

    br_http_server_parser_cb response_cb = (br_http_server_parser_cb) cli->m_server->m_gen_response_cb;
    MM_ASSERT(0 == response_cb(cli));

    uv_write(
            &cli->m_write_req,
            (uv_stream_t*) & cli->m_handle,
            &cli->m_resbuf,
            1,
            on_http_after_write);
    return 1;
}

static void on_http_read(uv_stream_t* handle_, ssize_t nread_, const uv_buf_t* buf_) {
    size_t parsed;
    br_http_client_t* client = (br_http_client_t*) handle_->data;
    br_http_server_t* server = client->m_server;

    if (nread_ >= 0) {
        parsed = http_parser_execute(
                &client->m_parser, &server->m_parser_settings, buf_->base, (size_t) nread_);

        if (parsed < (size_t) nread_) {
            MM_ERR("parse error");
            uv_close((uv_handle_t*) handle_, on_http_close);
        }

    } else {
        uv_close((uv_handle_t*) handle_, on_http_close);
    }
    free(buf_->base);
}

static void on_http_connect(uv_stream_t* handle_, int status_) {
    MM_ASSERT(status_ >= 0);
    br_http_server_t* server = (br_http_server_t*) handle_->data;

    ++(server->m_request_num);

    br_http_client_t* cli = calloc(1, sizeof (br_http_client_t));
    cli->m_request_num = server->m_request_num;
    cli->m_server = server;

    MM_INFO("(%5d) connection new %p", cli->m_request_num, handle_);

    uv_tcp_init(uv_default_loop(), &cli->m_handle);
    http_parser_init(&cli->m_parser, HTTP_REQUEST);

    cli->m_parser.data = cli;
    cli->m_handle.data = cli;

    int r = uv_accept(handle_, (uv_stream_t*) & cli->m_handle);
    if (0 == r) {
        uv_read_start((uv_stream_t*) & cli->m_handle, on_alloc_buffer, on_http_read);
    } else {
        MM_ERR("connect:%s", uv_strerror(r));
        uv_close((uv_handle_t*) & cli->m_handle, on_http_close);
    }
}

int br_http_server_add(mmpool_t* srv_pool_, int port_, void* gen_response_cb_) {

    mmpool_item_t* pool_item = mmpool_take(srv_pool_);
    if (NULL == pool_item) return -1;
    br_http_server_t* server = (br_http_server_t*) pool_item->m_p;
    server->m_parser_settings.on_headers_complete = on_headers_complete;
    server->m_port = port_;
    server->m_gen_response_cb = gen_response_cb_;
    MM_INFO("(%d) http %d", server->m_port);
    uv_tcp_init(uv_default_loop(), &server->m_handler);
    server->m_handler.data = server;
    MM_ASSERT(0 == uv_ip4_addr("0.0.0.0", server->m_port, &server->m_addr));
    MM_ASSERT(0 == uv_tcp_bind(&server->m_handler, (const struct sockaddr*) &server->m_addr));
    MM_ASSERT(0 == uv_listen((uv_stream_t*) & server->m_handler, BR_MAX_CONNECTIONS, on_http_connect));
    return 0;
}

/**
 * timestamp reference
 */
static uv_timer_t __brtsref_req;
static unsigned __brtsref = 0;
static char __brtsrefhex[8 + 1] = "00000000";

static void on_tsref_update(uv_timer_t* handle, int status){
    (void)handle;
    (void)status;
    __brtsref = (unsigned)time(NULL);
    snprintf(__brtsrefhex,8 + 1,"%x",__brtsref);
}

void br_tsref_init(unsigned refresh_period_){
    uv_timer_init(uv_default_loop(), &__brtsref_req);
    uv_timer_start(&__brtsref_req, on_tsref_update, 0, refresh_period_);
}
unsigned br_tsref_get(){ return __brtsref;}
char* br_tsrefhex_get(){ return &__brtsrefhex[0];}

/**
 * common
 */
void br_run(void) {
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void br_stop(void) {
    uv_stop(uv_default_loop());
}
