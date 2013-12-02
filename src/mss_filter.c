#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


#include "uv.h"

#include "sad.h"
#include "bagride.h"
#include "mmtrace.h"

#define HELP_USAGE "usage: mss_filter cfg_file addr:port addr:port ... addr:port" 


static sad_filter_t filter;

struct mss_config_s {
    int ais_udp_in_port;
    int admin_http_port;
    double x1, y1, x2, y2; /* geo filter */
};


static struct mss_config_s cfg = {0};

static char last_sentence[1024] = {0};
static char forward_sentence[1024] = {0};

static br_udp_clients_t udp_clients = {0};

static int load_cfg(char *config_file_) {
    int r = 0;
    lua_State *L = luaL_newstate();
    luaopen_base(L);
    luaopen_io(L);
    luaopen_string(L);
    luaopen_math(L);

    if (luaL_loadfile(L, config_file_) || lua_pcall(L, 0, 0, 0)) {
        MM_ERR("cannot run configuration file: %s");
        r = -1;
        goto end;
    }

#define MM_GETINT_FROMG(NAME) \
    lua_getglobal(L, #NAME);\
    if (!lua_isnumber(L, -1)) {\
        MM_ERR(#NAME " should be a number\n");\
        r = -1; goto end;\
    }\
    cfg.NAME = (int) lua_tonumber(L, -1);\
    lua_pop(L, 1);     

    /*
     * ais_in_udp_port
     */
    MM_GETINT_FROMG(ais_udp_in_port);

    /*
     * admin_http_port
     */
    MM_GETINT_FROMG(admin_http_port);

    /*
     * geofilter 
     */
    lua_getglobal(L, "geofilter");
    if (!lua_istable(L, -1)) {
        r = -1;
        goto end;
    }

#define MM_GETDOUBLE_FROMTABLE(NAME) \
    lua_getfield(L, -1, #NAME);\
    if (!lua_isnumber(L, -1)) {\
        MM_ERR(#NAME " should be a number\n");\
        r = -1; goto end;\
    }\
    cfg.NAME = (double) lua_tonumber(L, -1);\
    lua_pop(L, 1);    

    MM_GETDOUBLE_FROMTABLE(x1)
    MM_GETDOUBLE_FROMTABLE(x2)
    MM_GETDOUBLE_FROMTABLE(y1)
    MM_GETDOUBLE_FROMTABLE(y2)

#undef MM_GETDOUBLE_FROMTABLE

    /*
     * out_filter 
     */
    lua_getglobal(L, "out_filter");
    if (!lua_istable(L, -1)) {
        r = -1;
        goto end;
    }    
    
         
    

    end :
            lua_close(L);
    return r;
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

static int on_ais_decoded(struct sad_filter_s * filter_) {

    struct ais_t * ais = &filter_->ais;
    sub0_substring_t* sentence = filter_->sentence;

    if (1 == ais->type || 2 == ais->type || 3 == ais->type) {

        if (0 == strncmp(last_sentence, sentence->start, sentence->n)) {
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

        const double lat = (double) ais->type1.lat / AIS_LATLON_DIV;
        const double lon = (double) ais->type1.lon / AIS_LATLON_DIV;

        if (lon > cfg.x1 && lon < cfg.x2 && lat < cfg.y1 && lat > cfg.y2) {

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

            br_udp_clients_send(&udp_clients, forward_sentence);
        }
    }
    return 0;
}

static void mss_info_error(void) {
    printf(HELP_USAGE "\n");
}

static br_http_server_t http_server = {.m_gen_response_cb = on_stats_response};
static br_udp_server_t udp_server = {.m_user_parse_cb = on_udp_parse};

int main(int argc, char **argv) {
    int r = 0;

#define MM_GERR { r=-1;mss_info_error();goto end;}

    if (2 > argc) MM_GERR
        if (0 > br_udp_clients_init(&udp_clients, argc - 2)) MM_GERR

            MM_INFO("config %s", argv[1]);
    if (0 > load_cfg(argv[1])) MM_GERR

        MM_INFO("geofilter(%f,%f,%f,%f)", cfg.x1, cfg.y1, cfg.x2, cfg.y2);

    int current_arg = 2;
    do {
        if (0 > br_udp_clients_add(&udp_clients, argv[current_arg])) MM_GERR
                ++current_arg;
    } while (current_arg < argc);

    if (sad_filter_init(&filter, on_ais_decoded, NULL)) MM_GERR

    udp_server.m_port = cfg.ais_udp_in_port;
    br_udp_server_register(&udp_server, 1);

    http_server.m_port = cfg.admin_http_port;
    br_http_server_register(&http_server, 1);

    br_run();

#undef MM_GERR

end:
    br_udp_clients_close(&udp_clients);

    return r;

}



