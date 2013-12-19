#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "uv.h"

#include "sad.h"
#include "bagride.h"
#include "mmtrace.h"

#include "ais_server_config.h"

#define HELP_USAGE "usage: ais_server cfg_file" 

static sad_filter_t server;

static struct ais_server_config_s config;

static char last_sentence[1024] = {0};
static char forward_sentence[1024] = {0};

static mmpool_t* udp_clients = NULL;

static mmpool_t* udp_servers = NULL;
static mmpool_t* http_servers = NULL;
static mmpool_t* tcp_servers = NULL;


static int on_stats_response(br_http_client_t* cli_) {
    cli_->m_resbuf.len = sad_stats_string(&cli_->m_resbuf.base, &server);
    return 0;
}

static int on_udp_parse(ssize_t nread_, const uv_buf_t* inbuf_, br_udp_server_t* pserver_) {
    (void) pserver_;
    sad_decode_multiline(&server, inbuf_->base, nread_);
    return 0;
}

static int on_ais_decoded(struct sad_filter_s * server_) {

    struct ais_t * ais = &server_->ais;
    sub0_substring_t* sentence = server_->sentence;

    if (1 == ais->type || 2 == ais->type || 3 == ais->type) {

        if (0 == strncmp(last_sentence, sentence->start, sentence->n)) {
            /* drop duplicate */
#ifndef NDEBUG            
            MM_INFO("drop duplicate %08" PRIu64 " type:%02d mmsi:%09u %s",
                    server_->sentences,
                    ais->type,
                    ais->mmsi,
                    server_->sentence->start);
#endif            
            return 0;
        }
        strncpy(last_sentence, sentence->start, sentence->n);
        last_sentence[sentence->n + 1] = '0';

        const double lat = (double) ais->type1.lat / AIS_LATLON_DIV;
        const double lon = (double) ais->type1.lon / AIS_LATLON_DIV;

        if (lon > config.geoserver.x1 && lon < config.geoserver.x2 
          && lat < config.geoserver.y1 && lat > config.geoserver.y2) {

            strncpy(forward_sentence, sentence->start, sentence->n);
            forward_sentence[sentence->n] = '\n';
            forward_sentence[sentence->n + 1] = '\0';

#ifdef MM_ULTRADEBUG
            MM_INFO("%08" PRIu64 " type:%02d mmsi:%09u lat:%f lon:%f %s",
                    server_->sentences, ais->type, ais->mmsi,lat,lon,forward_sentence);
#endif /* MM_ULTRADEBUG */

            if(0 < config.ais_out_udp.n){
              br_udp_clients_send(udp_clients, forward_sentence);
            }
            
            br_tcp_write_string((br_tcp_server_t*)(tcp_servers->items[0]->m_p), 
                    forward_sentence, sentence->n + 1);
        }
    }
    return 0;
}

static int on_tcp_parse(ssize_t nread_, const uv_buf_t* inbuf_, br_tcp_server_t* pserver_) {
    MM_INFO("tcp (%d)[%d] %s data (%p)", pserver_->m_port, (int) nread_, inbuf_->base,
            pserver_->m_data);
    if (0 > asprintf(&pserver_->m_write_buffer.base, "<<%s>>", inbuf_->base)) return -1;
    pserver_->m_write_buffer.len = strlen(pserver_->m_write_buffer.base);
    return 0;
}

static void mss_info_error(void) {
    printf(HELP_USAGE "\n");
}

int main(int argc, char **argv) {
    int r = 0;
    
#define MM_GERR { r=-1;mss_info_error();goto end;}

    if (2 > argc) MM_GERR;

    MM_INFO("version=\"%s\"", MM_VERSION_INFO );
    MM_INFO("exe=\"%s\"", argv[0]);
    MM_INFO("config=\"%s\"", argv[1]);
    if (0 > ais_server_config_load(&config,argv[1])) MM_GERR;

    MM_INFO("geoserver={x1=%f,y1=%f,x2=%f,y2=%f}", config.geoserver.x1, 
            config.geoserver.y1, config.geoserver.x2, config.geoserver.y2);
    
    /* udp client init */
    if(0 < config.ais_out_udp.n)
    {
        if (NULL == (udp_clients = mmpool_new(config.ais_out_udp.n, sizeof(br_udp_client_t), NULL))) return -1;
        int idx;
        for(idx=0;idx<config.ais_out_udp.n;idx++){
          const char* s = config.ais_out_udp.items[idx];
            if (0 > br_udp_client_add(udp_clients, s)) MM_GERR;
            MM_INFO("ais_out_udp[%d]=\"%s\"", (idx+1),s);
        }
    }
        
    if (sad_filter_init(&server, on_ais_decoded, NULL)) MM_GERR;

    /* udp servers  */
    {
        if (NULL == (udp_servers = mmpool_new(1, sizeof(br_udp_server_t), NULL))) return -1;
        br_udp_server_add(udp_servers, config.ais_udp_in_port, on_udp_parse);
    }       
    
    /* http servers  */
    {
        if (NULL == (http_servers = mmpool_new(1, sizeof(br_http_server_t), NULL))) return -1;
        br_http_server_add(http_servers, config.admin_http_port, on_stats_response);
    }
    
    /* tcp servers  */
    {
        if (NULL == (tcp_servers = mmpool_new(1, sizeof(br_tcp_server_t), NULL))) return -1;
        br_tcp_server_add(tcp_servers,
                          config.ais_tcp_server.name, 
                          config.ais_tcp_server.port, 
                          on_tcp_parse,
                          config.ais_tcp_server.max_connections);
    }    
    br_run();

#undef MM_GERR

end:

    /* cleaning */
    {
        mmpool_free(udp_clients);
        mmpool_free(udp_servers);
        mmpool_free(http_servers);
        br_tcp_servers_close(tcp_servers);
        ais_server_config_close(&config);
    }    


    return r;

}



