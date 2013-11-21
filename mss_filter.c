#include <stdlib.h>
#include <string.h>
#include "uv.h"

#include "sad.h"
#include "br_net.h"
#include "mmtrace.h"



#define HELP_USAGE "usage: mss_filter addr port" 
#define UDP_AIS_PORT 9998
#define HTTP_AIS_PORT 9997


static sad_filter_t filter;

/* geographical filter */
static double x1 = -16.0;
static double y1 = 45.0;
static double x2 = 9.0;
static double y2 = 36.0;

static char last_sentence[1024] = {0};
static char forward_sentence[1024] = {0};

static uv_udp_t send_socket;
static struct sockaddr_in send_addr;

static void on_send(uv_udp_send_t* req_, int status){
  (void)status;
  char *base = (char*) req_->data;
  free(base);
  free(req_);
}

static int on_ais_decoded(struct sad_filter_s * filter_){
    
    struct ais_t * ais = &filter_->ais;
    sub0_substring_t* sentence = filter_->sentence;
    
    if(1 == ais->type || 2 == ais->type  || 3 == ais->type ){
        
        if(0 == strncmp(last_sentence, sentence->start, sentence->n)){
            /* drop duplicate */
#ifdef DEBUG            
            printf("[KO] drop duplicate %08" PRIu64 " type:%02d mmsi:%09u %s", 
            filter_->sentences, 
            ais->type, 
            ais->mmsi,
            filter_->sentence->start);
#endif            
            return 0;
        }
        
        strncpy(last_sentence, sentence->start, sentence->n);
        last_sentence[sentence->n + 1] = '0'; 
        
        const double lat = (double)ais->type1.lat / AIS_LATLON_DIV;
        const double lon = (double)ais->type1.lon / AIS_LATLON_DIV;
        
        if(lon > x1 && lon < x2 && lat < y1 && lat > y2){
	  
	  strncpy(forward_sentence, sentence->start, sentence->n);
	  forward_sentence[sentence->n] = '\n';
	  forward_sentence[sentence->n + 1] = '\0';
	  
            printf("[ok] %08" PRIu64 " type:%02d mmsi:%09u lat:%f lon:%f %s", 
            filter_->sentences, 
            ais->type, 
            ais->mmsi,
            lat,
            lon,
            forward_sentence);
	    
	    
	    uv_buf_t udp_sentence;
	    uv_udp_send_t* send_req = (uv_udp_send_t*)calloc(1, sizeof(uv_udp_send_t));
	    
	    udp_sentence.base = (char*) strdup(forward_sentence);
	    udp_sentence.len = sentence->n + 1;
	    
	    uv_udp_send(send_req, &send_socket, &udp_sentence, 1, (const struct sockaddr *)&send_addr, on_send);                   
                          
	    
	}

    }
    return 0;
}

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
        .m_port = HTTP_AIS_PORT,
        .m_gen_response_cb = on_stats_response
    },
};

static void mss_info_error(void){
  printf(HELP_USAGE "\n");
}

int main(int argc, char **argv) { 

    if (3 != argc) goto err;
    int port = atoi(argv[2]);
    
    
    MM_INFO("init %s:%d", argv[1], port);  
    
    if (sad_filter_init(&filter, on_ais_decoded, NULL)) goto err;
    
    br_udp_register(udp_servers, sizeof (udp_servers) / sizeof (br_udp_server_t));
    br_http_register(http_servers, sizeof (http_servers) / sizeof (br_http_server_t));
    
    /* register udp send */
    {
       uv_udp_init(uv_default_loop(), &send_socket);
       if(0 > uv_ip4_addr(argv[1], port, &send_addr)) goto err;
       uv_udp_bind(&send_socket, (const struct sockaddr*)(&send_addr), 0);
       MM_INFO("[ ok ] udp bind");
    } 
    
    br_run();

    return 0;
    
 err:   
    mss_info_error();
    return 1;

}



