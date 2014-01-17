#include <stdlib.h>
#include <string.h>

#include "sad.h"
#include "bagride.h"
#include "mmtrace.h"
#include "mmconfig.h"
#include "mmvector.h"
#include "netchannel.h"

#define MM_VERSION_INFO "v0.2a"

#define HELP_USAGE "usage: ais_filter cfg_file" 

#define AIS_SRV       0
#define AIS_SRV_ERROR 1

#define TIMESTAMP_UPDATE_MS 1000

typedef struct {
    const char* addr;
    int port;
} out_ais_udp_t;

typedef struct {
    const char* name;
    double x1;
    double y1;
    double x2;
    double y2;
} geofilter_t;

static struct {
    int io_admin_http_port;
    channel_in_values_t in_ais;
    channel_out_values_t out_ais_tcp_server;
    channel_out_values_t out_ais_tcp_error;
    MM_DECL_VECTOR(geofilter_t) geofilters;
    MM_DECL_VECTOR(out_ais_udp_t) out_ais_udp_streams;
} values;

static sad_filter_t filter;

static mmpool_t* udp_clients = NULL;

static br_udp_server_t in_raw_ais_udpsrv;
static br_tcp_client_t in_raw_ais_tcpclient;

static br_http_server_t srv_out_http_stats;
static br_tcp_server_t srv_out_tcp_filtered_ais;
static br_tcp_server_t srv_out_tcp_filtered_ais_error;

static int on_stats_response(br_http_client_t* cli_) {
    cli_->m_resbuf.len = sad_stats_string(&cli_->m_resbuf.base, &filter);
    return 0;
}

static int on_ais_raw_udpserver(ssize_t nread_, const br_buf_t* inbuf_, br_udp_server_t* pserver_) {
    (void) pserver_;
    sad_decode_multiline(&filter, inbuf_->base, nread_);
    return 0;
}

static int on_ais_raw_tcpclient(ssize_t nread_, const br_buf_t* inbuf_,
        br_tcp_client_t* client_) {
    (void) client_;
    sad_decode_multiline(&filter, inbuf_->base, nread_);
    return 0;
}

static int on_ais_decoded(struct sad_filter_s * f_) {

    struct ais_t * ais = &f_->ais;
    sub0_substring_t* s = f_->mess;

    if (3u < ais->type) return 0;

    const double lat = (double) ais->type1.lat * AIS_LATLON_DIV_INV;
    const double lon = (double) ais->type1.lon * AIS_LATLON_DIV_INV;

    const geofilter_t* gf = values.geofilters.items;
    unsigned n = values.geofilters.n;

    do {
        if (lon > gf->x1 && lon < gf->x2 && lat < gf->y1 && lat > gf->y2)
            goto found;
        ++gf;
        --n;
    } while (n);

    return 0;

found:{
        const char* hx = br_tsrefhex_get();
        char* fwd_without_ts = &f_->fwd_mess[MM_HEX_TIMESTAMP_LEN];

        const size_t fwd_len_without_ts = s->n + 1;
        const size_t fwd_with_ts_len = fwd_len_without_ts + MM_HEX_TIMESTAMP_LEN;
        strncpy(f_->fwd_mess, hx, MM_HEX_TIMESTAMP_LEN + 1);
        strncpy(fwd_without_ts, s->start, s->n);
        f_->fwd_mess[fwd_with_ts_len - 1] = '\n';
        f_->fwd_mess[fwd_with_ts_len] = '\0';

        /* for udp !aivdm forwarded without hex timestamp */
        if (0 < values.out_ais_udp_streams.n) {
            br_udp_clients_send(udp_clients, fwd_without_ts, fwd_len_without_ts);
        }

        /* only if we have clients */
        if (0 == mmpool_taken_len(srv_out_tcp_filtered_ais.m_clients)) return 0;

        /* only hex timestamp is handled for now*/
        if (NC_TS_HEX == values.out_ais_tcp_server.ts_id) {
            br_out_tcp_write_string(&srv_out_tcp_filtered_ais, f_->fwd_mess, fwd_with_ts_len);
        } else {
            br_out_tcp_write_string(&srv_out_tcp_filtered_ais, fwd_without_ts, fwd_len_without_ts);
        }

        return 0;
    }
}

static int on_tcp_parse(ssize_t nread_, const uv_buf_t* inbuf_, br_tcp_server_t* pserver_) {
    MM_INFO("tcp (%d)[%d] %s data (%p)\n", pserver_->m_port, (int) nread_,
            inbuf_->base, pserver_->m_data);
    if (0 > asprintf(&pserver_->m_write_buffer.base, "<<%s>>", inbuf_->base)) return -1;
    pserver_->m_write_buffer.len = strlen(pserver_->m_write_buffer.base);
    return 0;
}

static void ais_decode_error(const char* errm_) {
    if (0 == mmpool_taken_len(srv_out_tcp_filtered_ais_error.m_clients)) return;
    br_out_tcp_write_string(&srv_out_tcp_filtered_ais_error, errm_, strlen(errm_));
}

static int load_config(config_t* cfg_) {
    int r = 0;

    MM_CFG_GET_STR(cfg_, in_ais.type, values);
    MM_CFG_GET_STR(cfg_, in_ais.name, values);
    MM_CFG_GET_INT(cfg_, in_ais.port, values);

    values.in_ais.type_id = nc_in_channel_type_id(values.in_ais.type);

    if (NC_IN_INVALID == values.in_ais.type_id) {
        MM_ERR("Invalid in channel type:%s", values.in_ais.type);
        goto err;
    }
    /* need to get the addr if tcp client */
    if (NC_IN_TCPCLI == values.in_ais.type_id) {
        MM_CFG_GET_STR(cfg_, in_ais.addr, values);
    }

    if (!(NC_IN_TCPCLI == values.in_ais.type_id || NC_IN_UDPSRV == values.in_ais.type_id)) {
        MM_ERR("Invalid in channel type:%s, expected %s or %s",
                values.in_ais.type, NC_UDPSRV, NC_TCPCLI);
        goto err;
    }

    MM_CFG_GET_INT(cfg_, io_admin_http_port, values);

    MM_CFG_GET_STR(cfg_, out_ais_tcp_server.name, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_server.port, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_server.max_connections, values);

    MM_CFG_GET_STR(cfg_, out_ais_tcp_server.timestamp, values);
    values.out_ais_tcp_server.ts_id = nc_out_channel_ts_id(values.out_ais_tcp_server.timestamp);

    MM_CFG_GET_STR(cfg_, out_ais_tcp_error.name, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_error.port, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_error.max_connections, values);

    /* geofilters */
    {
        config_setting_t *list = config_lookup(cfg_, "geofilter");
        if (NULL == list) {
            MM_GERR("Missing geofilters in configuration file");
        }
        MM_ALLOC_VECTOR(values.geofilters, geofilter_t, config_setting_length(list));
        MM_INFO("geofilter len:%d", values.geofilters.n);
        geofilter_t* p;
        int i;

        for (i = 0, p = values.geofilters.items; i < values.geofilters.n; ++i, p++) {
            config_setting_t *setting = config_setting_get_elem(list, i);
            MM_CFGNODE_GET_STR(setting, name, p);
            MM_INFO("geofilter[%d].name == \"%s\"", i, p->name);
            MM_CFGNODE_GET_DOUBLE(setting, x1, p);
            MM_CFGNODE_GET_DOUBLE(setting, y1, p);
            MM_CFGNODE_GET_DOUBLE(setting, x2, p);
            MM_CFGNODE_GET_DOUBLE(setting, y2, p);
        }
    }
    /* out_ais_udp */
    {
        config_setting_t *list = config_lookup(cfg_, "out_ais_udp");
        if (NULL == list) {
            MM_INFO("No ais udp output");
        } else {
            MM_ALLOC_VECTOR(values.out_ais_udp_streams, out_ais_udp_t, config_setting_length(list));
            MM_INFO("out_ais_udp_streams len:%d", values.out_ais_udp_streams.n);
            out_ais_udp_t* p;
            int i;
            for (i = 0, p = values.out_ais_udp_streams.items; i < values.out_ais_udp_streams.n; ++i, p++) {
                config_setting_t *setting = config_setting_get_elem(list, i);
                MM_CFGNODE_GET_STR(setting, addr, p);
                MM_INFO("out_ais_udp[%d].addr == %s", i, p->addr);
                MM_CFGNODE_GET_INT(setting, port, p);
            }
        }
    }

end:
    return r;

err:
    r = -1;
    goto end;
}

int main(int argc, char **argv) {
    int r = 0;
    MM_INFO("start %s", argv[0]);

    br_tsref_init(TIMESTAMP_UPDATE_MS);

    if (2 > argc) MM_GERR(HELP_USAGE);

    MM_INFO("version=\"%s\"", MM_VERSION_INFO);
    MM_INFO("conf=\"%s\"", argv[1]);

    config_t cfg;
    memset(&values, 0, sizeof (values));
    config_init(&cfg);

    if (!config_read_file(&cfg, argv[1])) {
        MM_GERR("%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
    }
    if (0 < load_config(&cfg)) MM_GERR("Error in configuration file %s", argv[1]);

    /* udp client init */
    if (0 < values.out_ais_udp_streams.n) {
        if (NULL == (udp_clients = mmpool_easy_new(values.out_ais_udp_streams.n, sizeof (br_udp_client_t), NULL))) return -1;
        int idx;
        out_ais_udp_t* p = values.out_ais_udp_streams.items;
        for (idx = 0; idx < values.out_ais_udp_streams.n; idx++, p++) {
            if (0 > br_udp_client_add(udp_clients, p->addr, p->port)) {
                MM_GERR("invalid udp client %s:%d", p->addr, p->port);
            }
            MM_INFO("ais_out_udp[%d]=\"%s:%d\"", (idx + 1), p->addr, p->port);
        }
    }
    if (sad_filter_init(&filter, on_ais_decoded, NULL, ais_decode_error))
        MM_GERR("internal error");

    /* udp server  */
    if (NC_IN_UDPSRV == values.in_ais.type_id) {
        br_udp_server_init(&in_raw_ais_udpsrv, values.in_ais.port, on_ais_raw_udpserver);
    } else {
        /* tcp client */
        br_tcp_client_init(&in_raw_ais_tcpclient, values.in_ais.name,
                values.in_ais.addr, values.in_ais.port, on_ais_raw_tcpclient);
    }

    /* http server  */
    br_http_server_init(&srv_out_http_stats, values.io_admin_http_port, on_stats_response);

    /* tcp servers  */
    br_tcp_server_init(&srv_out_tcp_filtered_ais,
            values.out_ais_tcp_server.name,
            values.out_ais_tcp_server.port,
            on_tcp_parse,
            values.out_ais_tcp_server.max_connections);

    br_tcp_server_init(&srv_out_tcp_filtered_ais_error,
            values.out_ais_tcp_error.name,
            values.out_ais_tcp_error.port,
            on_tcp_parse,
            values.out_ais_tcp_error.max_connections);

    br_run();


end:
    mmpool_free(udp_clients);
    br_tcp_server_close(&srv_out_tcp_filtered_ais);
    br_tcp_server_close(&srv_out_tcp_filtered_ais_error);
    config_destroy(&cfg);
    MM_FREE_VECTOR(values.geofilters);
    MM_FREE_VECTOR(values.out_ais_udp_streams);
    return r;

err:
    r = -1;
    goto end;
}



