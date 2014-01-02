#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "uv.h"

#include "sad.h"
#include "bagride.h"
#include "mmtrace.h"

#include "ais_filter_config.h"

#define HELP_USAGE "usage: ais_filter cfg_file" 

#define AIS_SRV       0
#define AIS_SRV_ERROR 1

static sad_filter_t filter;

static struct ais_filter_config_s conf;

static mmpool_t* udp_clients = NULL;

static mmpool_t* udp_servers = NULL;
static mmpool_t* http_servers = NULL;
static mmpool_t* tcp_servers = NULL;


static void user_info_dump(void) {
    printf(HELP_USAGE "\n");
}

static int on_stats_response(br_http_client_t* cli_) {
    cli_->m_resbuf.len = sad_stats_string(&cli_->m_resbuf.base, &filter);
    return 0;
}

static int on_udp_parse(ssize_t nread_, const uv_buf_t* inbuf_, br_udp_server_t* pserver_) {
    (void) pserver_;
    sad_decode_multiline(&filter, inbuf_->base, nread_);
    return 0;
}

static int on_ais_decoded(struct sad_filter_s * f_) {

    struct ais_t * ais = &f_->ais;
    sub0_substring_t* sentence = f_->sentence;

    if (3 < ais->type ) return 0;

    const double lat = (double) ais->type1.lat / AIS_LATLON_DIV;
    const double lon = (double) ais->type1.lon / AIS_LATLON_DIV;

    if (lon > conf.geofilter.x1 && lon < conf.geofilter.x2
            && lat < conf.geofilter.y1 && lat > conf.geofilter.y2) {

        strncpy(f_->forward_sentence, sentence->start, sentence->n);
        f_->forward_sentence[sentence->n] = '\n';
        f_->forward_sentence[sentence->n + 1] = '\0';

#ifdef MM_ULTRADEBUG
        MM_INFO("%08" PRIu64 " type:%02d mmsi:%09u lat:%f lon:%f %s",
                f_->sentences, ais->type, ais->mmsi, lat, lon, forward_sentence);
#endif /* MM_ULTRADEBUG */

        if (0 < conf.ais_out_udp.n) {
            br_udp_clients_send(udp_clients, f_->forward_sentence);
        }

        br_tcp_write_string((br_tcp_server_t*) (tcp_servers->items[AIS_SRV].m_p),
                f_->forward_sentence, sentence->n + 1);
    }
    return 0;
}

static int on_tcp_parse(ssize_t nread_, const uv_buf_t* inbuf_, br_tcp_server_t* pserver_) {
    MM_INFO("tcp (%d)[%d] %s data (%p)\n", pserver_->m_port, (int) nread_, inbuf_->base,
            pserver_->m_data);
    if (0 > asprintf(&pserver_->m_write_buffer.base, "<<%s>>", inbuf_->base)) return -1;
    pserver_->m_write_buffer.len = strlen(pserver_->m_write_buffer.base);
    return 0;
}



static void ais_decode_error(const char* errm_){
  br_tcp_server_t* server = (br_tcp_server_t*) (tcp_servers->items[AIS_SRV_ERROR].m_p);
  br_tcp_write_string(server, errm_, strlen(errm_));
}

int main(int argc, char **argv) {
    int r = 0;
    MM_INFO("start %s", argv[0]);
    
#define MM_GERR { r=-1;user_info_dump();goto end;}

    if (2 > argc) MM_GERR;

    MM_INFO("version=\"%s\"", MM_VERSION_INFO );
    MM_INFO("conf=\"%s\"", argv[1]);
    if (0 > ais_filter_config_load(&conf,argv[1])) MM_GERR;

    MM_INFO("geofilter={x1=%f,y1=%f,x2=%f,y2=%f}", conf.geofilter.x1, 
            conf.geofilter.y1, conf.geofilter.x2, conf.geofilter.y2);
    
    /* udp client init */
    if(0 < conf.ais_out_udp.n)
    {
        if (NULL == (udp_clients = mmpool_easy_new(conf.ais_out_udp.n, sizeof(br_udp_client_t), NULL))) return -1;
        int idx;
        for(idx=0;idx<conf.ais_out_udp.n;idx++){
          const char* s = conf.ais_out_udp.items[idx];
            if (0 > br_udp_client_add(udp_clients, s)) MM_GERR;
            MM_INFO("ais_out_udp[%d]=\"%s\"", (idx+1),s);
        }
    }
        
    if (sad_filter_init(&filter, on_ais_decoded, NULL,ais_decode_error)) MM_GERR;

    /* udp servers  */
    {
        if (NULL == (udp_servers = mmpool_easy_new(1, sizeof(br_udp_server_t), NULL))) return -1;
        br_udp_server_add(udp_servers, conf.ais_udp_in_port, on_udp_parse);
    }       
    
    /* http servers  */
    {
        if (NULL == (http_servers = mmpool_easy_new(1, sizeof(br_http_server_t), NULL))) return -1;
        br_http_server_add(http_servers, conf.admin_http_port, on_stats_response);
    }
    
    /* tcp servers  */
    {
        if (NULL == (tcp_servers = mmpool_easy_new(2, sizeof(br_tcp_server_t), NULL))) return -1;
        br_tcp_server_add(tcp_servers,
                          conf.ais_tcp_server.name, 
                          conf.ais_tcp_server.port, 
                          on_tcp_parse,
                          conf.ais_tcp_server.max_connections);
        
        br_tcp_server_add(tcp_servers,
                          conf.ais_tcp_error.name, 
                          conf.ais_tcp_error.port, 
                          on_tcp_parse,
                          conf.ais_tcp_error.max_connections);        
        
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
        ais_filter_config_close(&conf);
    }    


    return r;

}



