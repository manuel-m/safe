#ifndef BR_NET_H
#define	BR_NET_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "uv.h"
#include "http_parser.h"
    
#include "mmpool.h"    
    
#define br_buf_t uv_buf_t
#define br_tcp_t uv_tcp_t
#define br_udp_t uv_udp_t
    
#define BR_MAX_CONNECTIONS 64
#define BR_MAX_ADDR_SIZE 2048
   
   
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
    mmpool_t* m_clients;
} br_tcp_server_t;

void br_tcp_servers_close(mmpool_t* srv_pool_);

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


/**
 * @param cli_pool_item
 *   mmpool_item_t instead of handle
 *    to rollback if needed (bad DNS resolution)
 * @return 0 on success
 */
int br_udp_client_register(mmpool_item_t* cli_pool_item_);

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

        
int br_udp_client_add(mmpool_t* cli_pool_, const char* target_);
void br_udp_clients_send(mmpool_t* cli_pool_, const char* str_);

int br_udp_server_add(mmpool_t* serv_pool_, int port_, void* user_parse_cb_);

int br_tcp_server_add(mmpool_t* serv_pool_, const char* name_, int port_, 
        void* user_parse_cb_, int max_connections_);

int br_http_server_add(mmpool_t*  serv_pool_, int port_, void* gen_response_cb_);


/**
 * timestamp
 */
void br_tsref_init();
unsigned br_tsref_get();
char* br_tsrefhex_get();

/**
 * common
 */
void br_run(void);
void br_stop(void);

#ifdef	__cplusplus
}
#endif

#endif	/* BR_NET_H */

