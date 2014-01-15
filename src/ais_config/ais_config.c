
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmconfig.h"
#include "mmtrace.h"
#include "mmvector.h"

typedef struct {
    const char* name;
    double x1;
    double y1;
    double x2;
    double y2;
} geofilter_t;

typedef struct {
    const char* addr;
    int port;
} out_ais_udp_t;

static struct {
    int io_admin_http_port;
    int in_ais_udp_port;

    struct {
        const char* name;
        int port;
        int max_connections;
    } out_ais_tcp_server;

    struct {
        const char* name;
        int port;
        int max_connections;
    } out_ais_tcp_error;

    MM_DECL_VECTOR(geofilter_t) geofilters;
    MM_DECL_VECTOR(out_ais_udp_t) out_ais_udp_streams;

} values;

static int load_config(config_t* cfg_){
 int r = 0;
  
    /* io_admin_http_port */
    MM_CFG_GET_INT(cfg_, io_admin_http_port, values);

    /* in_ais_udp_port */
    MM_CFG_GET_INT(cfg_, in_ais_udp_port, values);

    /* out_ais_tcp_server */
    MM_CFG_GET_STR(cfg_, out_ais_tcp_server.name, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_server.port, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_server.max_connections, values);

    /* out_ais_tcp_error */
    MM_CFG_GET_STR(cfg_, out_ais_tcp_error.name, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_error.port, values);
    MM_CFG_GET_INT(cfg_, out_ais_tcp_error.max_connections, values);

    /* geofilters */
    {
        config_setting_t *list = config_lookup(cfg_, "geofilter");
        if (NULL == list) {
            MM_ERR("Missing geofilters in configuration file");
            goto err;
        }
        MM_ALLOC_VECTOR(values.geofilters, geofilter_t,config_setting_length(list));
        MM_INFO("geofilter len:%d",values.geofilters.n);
        geofilter_t* p;
        int i;
        
        for (i = 0, p = values.geofilters.items; i < values.geofilters.n; ++i, p++) {
            config_setting_t *setting = config_setting_get_elem(list, i);
            MM_CFGNODE_GET_STR(setting,name,p);  
            MM_INFO("geofilter[%d].name == %s",i, p->name);
            MM_CFGNODE_GET_DOUBLE(setting,x1,p);
            MM_CFGNODE_GET_DOUBLE(setting,y1,p);
            MM_CFGNODE_GET_DOUBLE(setting,x2,p);
            MM_CFGNODE_GET_DOUBLE(setting,y2,p);            
        }
    }
    
    /* out_ais_udp */
    {
        config_setting_t *list = config_lookup(cfg_, "out_ais_udp");
        if (NULL == list) {
            MM_ERR("Missing out_ais_udp in configuration file");
            goto err;
        }
        MM_ALLOC_VECTOR(values.out_ais_udp_streams, out_ais_udp_t,config_setting_length(list));
        
        MM_INFO("geofilter len:%d",values.geofilters.n);

        out_ais_udp_t* p;
        int i;
        
        for (i = 0, p = values.out_ais_udp_streams.items; i < values.out_ais_udp_streams.n; ++i, p++) {
            config_setting_t *setting = config_setting_get_elem(list, i);
            MM_CFGNODE_GET_STR(setting,addr,p);
            MM_INFO("out_ais_udp[%d].addr == %s",i, p->addr);
            MM_CFGNODE_GET_INT(setting,port,p);
        }
    }    
 
end:
    return r;

err:
    r = -1;
    goto end;  
}

int main(int argc, char **argv) {
  
    if (2 != argc) {
        MM_ERR("usage: %s /path/to/config/file", argv[0]);
        return -1;
    }
    int r = 0;
    config_t cfg;
    memset(&values,0,sizeof(values));

    config_init(&cfg);

    if (!config_read_file(&cfg, argv[1])) {
        MM_ERR("%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        goto err;
    }
    if(0 > load_config(&cfg)) goto end;
    
end:
    config_destroy(&cfg);
    MM_FREE_VECTOR(values.geofilters);
    MM_FREE_VECTOR(values.out_ais_udp_streams);
    return r;

err:
    r = -1;
    goto end;
    
    
}


