#include <stdlib.h>
#include <string.h>
#include "bagride.h"
#include "mmtrace.h"

static void on_alloc_buffer(uv_handle_t *handle_, size_t suggested_size_,
        uv_buf_t* buf_) {
    (void) handle_;
    buf_->base = (char*) calloc(suggested_size_, sizeof (char));
    buf_->len = suggested_size_;
}

static void on_close(uv_handle_t* handle_) {
    MM_INFO("connection close");
    free(handle_);
}

static void on_after_write(uv_write_t* req_, int status_) {
    MM_ASSERT(status_ >= 0);
    char *base = (char*) req_->data;
    free(base);
    free(req_);
}

static void on_tcp_read(uv_stream_t* stream_, ssize_t nread_, const uv_buf_t* read_buf_) {
    if (nread_ > 0) {
        br_tcp_server_t* tcp_server = (br_tcp_server_t*) stream_->data;
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

static void on_udp_send(uv_udp_send_t* req_, int status){
  (void)status;
  char *base = (char*) req_->data;
  free(base);
  free(req_);
}

static void on_connect(uv_stream_t* pserver_, int status_) {
    MM_ASSERT(status_ >= 0);
    uv_tcp_t *pclient = (uv_tcp_t*) calloc(1, sizeof (uv_tcp_t));
    uv_tcp_init(uv_default_loop(), pclient);
    pclient->data = pserver_->data;

    if (uv_accept(pserver_, (uv_stream_t*) pclient) == 0) {
        uv_read_start((uv_stream_t*) pclient, on_alloc_buffer, on_tcp_read);
    } else {
        uv_close((uv_handle_t*) pclient, on_close);
    }
}

void br_tcp_server_register(br_tcp_server_t* tcp_servers_, int size_){
    int i = 0;
    for (i = 0; i < size_; i++) {
        br_tcp_server_t* server = &tcp_servers_[i];
        MM_INFO("(%d) tcp listening on port %d", i, server->m_port);
        uv_tcp_init(uv_default_loop(), &server->m_handler);
        server->m_handler.data = server;
        MM_ASSERT(0 == uv_ip4_addr("0.0.0.0", server->m_port, &server->m_socketaddr));
        MM_ASSERT(0 == uv_tcp_bind(&server->m_handler, (const struct sockaddr*) &server->m_socketaddr));
        MM_ASSERT(0 == uv_listen((uv_stream_t*) & server->m_handler, BR_MAX_CONNECTIONS, on_connect));
    }
}

void br_udp_server_register(br_udp_server_t* udp_servers_, int size_) {
    int i = 0;
    for (i = 0; i < size_; i++) {
        br_udp_server_t* server = &udp_servers_[i];
        MM_INFO("(%d) udp listening on port %d", i, server->m_port);
        uv_udp_init(uv_default_loop(), &server->m_handler);
        server->m_handler.data = server;
        MM_ASSERT(0 == uv_ip4_addr("0.0.0.0", server->m_port, &server->m_socketaddr));
        MM_ASSERT(0 == uv_udp_bind(&server->m_handler, (const struct sockaddr*) &server->m_socketaddr, 0));
        MM_ASSERT(0 == uv_udp_recv_start(&server->m_handler, on_alloc_buffer, on_udp_recv));
    }
}

void br_udp_clientKO_register(br_udp_client_t* udp_clients_, int size_) {
    int i = 0;
    for (i = 0; i < size_; i++) {
        br_udp_client_t* client = &udp_clients_[i];     
        MM_INFO("(%d) udp sending on %s:%d", i, client->m_addr, client->m_port);
        uv_udp_init(uv_default_loop(), &client->m_handler);
        
        client->m_handler.data = client;
        MM_ASSERT(0 == uv_ip4_addr(client->m_addr, client->m_port, &client->m_socketaddr));
        MM_ASSERT(0 == uv_udp_bind(&client->m_handler, (const struct sockaddr*) &client->m_socketaddr, 0));      
    }
}

int br_udp_client_register(br_udp_client_t* cli_) {
    uv_udp_init(uv_default_loop(), &cli_->m_handler);
    if (0 > uv_ip4_addr(cli_->m_addr, cli_->m_port, &cli_->m_socketaddr)) return -1;
    uv_udp_bind(&cli_->m_handler, (const struct sockaddr*) (&cli_->m_socketaddr), 0);
    MM_INFO("[ ok ] udp bind");
    return 0;
}


void br_udp_client_send(br_udp_client_t* cli_, const char* str_) {
    uv_buf_t udp_sentence;
    uv_udp_send_t* send_req = (uv_udp_send_t*) calloc(1, sizeof (uv_udp_send_t));
    udp_sentence.base = (char*) strdup(str_);
    udp_sentence.len = strlen(str_) + 1;
    send_req->data = udp_sentence.base; /* no memory leak */
    uv_udp_send(send_req, &cli_->m_handler, &udp_sentence, 1, 
            (const struct sockaddr *) &cli_->m_socketaddr, on_udp_send);
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

    if (uv_accept(handle_, (uv_stream_t*) & cli->m_handle) == 0) {
        uv_read_start((uv_stream_t*) & cli->m_handle, on_alloc_buffer, on_http_read);
    } else {
        uv_close((uv_handle_t*) & cli->m_handle, on_http_close);
    }
}

void br_http_server_register(br_http_server_t* servers_, int size_) {
    int i = 0;
    for (i = 0; i < size_; i++) {
        br_http_server_t* server = &servers_[i];
        server->m_parser_settings.on_headers_complete = on_headers_complete;
        MM_INFO("(%d) http listening on port %d", i, server->m_port);
        uv_tcp_init(uv_default_loop(), &server->m_handler);
        server->m_handler.data = server;
        MM_ASSERT(0 == uv_ip4_addr("0.0.0.0", server->m_port, &server->m_addr));
        MM_ASSERT(0 == uv_tcp_bind(&server->m_handler, (const struct sockaddr*) &server->m_addr));
        MM_ASSERT(0 == uv_listen((uv_stream_t*) & server->m_handler, BR_MAX_CONNECTIONS, on_http_connect));
    }
}

/**
 * common
 */
void br_run(void) {
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void br_stop(void) {
    uv_stop(uv_default_loop());
}
