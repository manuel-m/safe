#ifndef BR_NET_H
#define	BR_NET_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "uv.h"
#include "http_parser.h"
    
    
#define br_buf_t uv_buf_t
#define br_tcp_t uv_tcp_t
#define br_udp_t uv_udp_t
    
#define BR_MAX_CONNECTIONS 64
#define BR_MAX_ADDR_SIZE 2048

#define BR_VECTOR_DECL(TYPE_ELEM) \
typedef struct TYPE_ELEM##s_s { \
    size_t n; /* total len */ \
    size_t i; /* current index */ \
    TYPE_ELEM##_t* items;\
} TYPE_ELEM##s_t; \
int TYPE_ELEM##s_init(TYPE_ELEM##s_t* uc_, size_t n_);\
void TYPE_ELEM##s_close(TYPE_ELEM##s_t* uc_);
    
   
/**
 * tcp
 **/    
typedef struct br_tcp_server_s {
    int m_port;
    struct sockaddr_in m_socketaddr;
    br_tcp_t m_server_handler;
    br_buf_t m_write_buffer;    
    void* m_user_parse_cb;
    void* m_data;
    const char* m_name;
    struct {
        int max_connections;
        int i; /* current index, init with -1 => no connection */
        br_tcp_t** items;
    } m_clients;
    
    
} br_tcp_server_t;


/**
 * parse incomming stream fragment
 * -> set write buffer if write required
 * -> return 0 on success
 */
typedef int (*br_tcp_server_parser_cb)(ssize_t nread_, const br_buf_t* pbuf_, 
        br_tcp_server_t* pserver_);


/**
 * udp
 **/    
typedef struct br_udp_server_s {
    br_udp_t m_handler;
    void* m_user_parse_cb;
    void* m_data;
    struct sockaddr_in m_socketaddr;
    int m_port;
} br_udp_server_t;


typedef struct br_udp_client_s {    
    int m_port;
    br_udp_t m_handler;
    struct sockaddr_in m_socketaddr;
    char m_addr[BR_MAX_ADDR_SIZE];
    
} br_udp_client_t;

typedef int (*br_udp_server_parser_cb)(ssize_t nread_, const br_buf_t* pbuf_, 
        br_udp_server_t* pserver_);

int br_udp_client_register(br_udp_client_t* cli_);

/**
 * http
 */
typedef struct br_http_server_s {
    struct sockaddr_in m_addr;
    uv_tcp_t m_handler;
    http_parser_settings m_parser_settings;
    int m_request_num;
    int m_port;
    void* m_gen_response_cb;
} br_http_server_t;

typedef struct br_http_client_s {
    uv_tcp_t m_handle;
    http_parser m_parser;
    uv_write_t m_write_req;
    int m_request_num;
    br_http_server_t* m_server;
    uv_buf_t m_resbuf;
} br_http_client_t;

int br_tcp_write_string(br_tcp_server_t*, const char* , size_t );


typedef int (*br_http_server_parser_cb)(br_http_client_t* cli_);

BR_VECTOR_DECL(br_tcp_server)
BR_VECTOR_DECL(br_http_server)
BR_VECTOR_DECL(br_udp_client)
BR_VECTOR_DECL(br_udp_server)
        
int br_udp_client_add(br_udp_clients_t* uc_, const char* target_);
void br_udp_clients_send(br_udp_clients_t* uc_, const char* str_);

int br_udp_server_add(br_udp_servers_t* uc_, int port_, void* user_parse_cb_);
int br_udp_server_add(br_udp_servers_t* uc_, int port_, void* user_parse_cb_);

int br_tcp_server_add(br_tcp_servers_t* uc_,
                      const char* name_, 
                      int port_, 
                      void* user_parse_cb_, 
                      int max_connections_);

int br_http_server_add(br_http_servers_t* uc_, int port_, void* gen_response_cb_);

/**
 * common
 */
void br_run(void);
void br_stop(void);

#ifdef	__cplusplus
}
#endif

#endif	/* BR_NET_H */

