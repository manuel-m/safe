#include <stdlib.h>
#include <string.h>

#include "sad.h"
#include "bagride.h"
#include "mmtrace.h"

#include "ais_server_config.h"

#define HELP_USAGE "usage: ais_server cfg_file" 

static sad_filter_t filter;

static struct ais_server_config_s config;

typedef struct mmship_s {
    unsigned mmsi;
    unsigned nb_update;
    type123_t ais;
} mmship_t;

static mmpool_t* live_ships = NULL;

static br_http_server_t srv_out_http_stats;
static br_tcp_server_t srv_out_ships;
static br_udp_server_t srv_in_ais;

static int on_stats_response(br_http_client_t* cli_) {
    cli_->m_resbuf.len = sad_stats_string(&cli_->m_resbuf.base, &filter);
    return 0;
}

static int on_udp_parse(ssize_t nread_, const uv_buf_t* inbuf_, br_udp_server_t* pserver_) {
    (void) pserver_;
    sad_decode_multiline(&filter, inbuf_->base, nread_);
    return 0;
}

static int mmsi_cmp_cb(void* l_, void* r_) {
    return (((mmship_t*)l_)->mmsi != (*(unsigned int*)r_)) ? 1 : 0;
}

static int on_ais_decoded(struct sad_filter_s * f_) {
    struct ais_t * ais = &f_->ais;
    int update = 0;

    if (3 < ais->type) return 0;
    
    mmpool_finder_t finder;
    mmpool_finder_init(finder, live_ships, mmsi_cmp_cb);


    mmpool_item_t * found_item = mmpool_find(&finder, (void*) (&ais->mmsi));
    mmship_t* ship = NULL;

    /* !found */
    if (NULL == found_item) {
        mmpool_item_t* item = mmpool_take(live_ships);
        if (item) {
            ship = (mmship_t*) item->m_p;
            ship->mmsi = ais->mmsi; 
       }
    } else {
        update = 1;
        ship = (mmship_t*) found_item->m_p;
        ++(ship->nb_update);
    }

    if (ship) {
        memcpy(&ship->ais, &(ais->type1), sizeof (type123_t));
    } else {
        MM_ERR("live ships buffer  overflow ...");
        return -1;
    }
    br_buf_t* buf = &srv_out_ships.m_write_buffer;

    if (0 == update) {
        buf->len = asprintf(&buf->base, "N %d\t(u:%d)[%d/%d]\n", ais->mmsi,ship->nb_update, 
                live_ships->m_taken_len,
                live_ships->m_alloc_len);
    } else {
        buf->len = asprintf(&buf->base, "u %d\t(u:%d)[%d/%d]\n", ais->mmsi,ship->nb_update, 
                live_ships->m_taken_len,
                live_ships->m_alloc_len);
    }

    if (0 > buf->len) return -1;

    br_out_tcp_write_string(&srv_out_ships, buf->base, buf->len);

    return 0;
}

static int on_tcp_parse(ssize_t nread_, const uv_buf_t* inbuf_, br_tcp_server_t* pserver_) {
    MM_INFO("tcp (%d)[%d] %s data (%p)", pserver_->m_port, (int) nread_, inbuf_->base,
            pserver_->m_data);
    if (0 > asprintf(&pserver_->m_write_buffer.base, "<<%s>>", inbuf_->base)) return -1;
    pserver_->m_write_buffer.len = strlen(pserver_->m_write_buffer.base);
    return 0;
}

static void ais_info_error(void) {
    printf(HELP_USAGE "\n");
}

int main(int argc, char **argv) {
    int r = 0;
    MM_INFO("start %s", argv[0]);

#define MM_GERR { r=-1;ais_info_error();goto end;}

    if (2 > argc) MM_GERR;

    MM_INFO("version=\"%s\"", MM_VERSION_INFO);
    
    MM_INFO("config=\"%s\"", argv[1]);

    if (0 > ais_server_config_load(&config, argv[1])) MM_GERR;

    /* live ships buffer init */
    if (NULL == (live_ships = mmpool_new(config.max_ships, config.max_ships, config.step_ships, sizeof (mmship_t), NULL))) {
         MM_ERR("too many requested ships: %d", config.max_ships);
         return -1;
     }

    if (sad_filter_init(&filter, on_ais_decoded, NULL,NULL)) MM_GERR;

    /* udp server  */
    br_udp_server_init(&srv_in_ais, config.ais_udp_in_port, on_udp_parse);

    /* http servers  */
    br_http_server_init(&srv_out_http_stats, config.admin_http_port, on_stats_response);

    /* tcp server  */
    br_tcp_server_init(&srv_out_ships,
                       config.ais_tcp_server.name,
                       config.ais_tcp_server.port,
                       on_tcp_parse,
                       config.ais_tcp_server.max_connections);
        
    br_run();

#undef MM_GERR

end:

    /* cleaning */
    {
        br_tcp_server_close(&srv_out_ships);
        ais_server_config_close(&config);
    }


    return r;

}



