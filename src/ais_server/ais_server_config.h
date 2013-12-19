#ifndef __AIS_FILTER_CONFIG
#define __AIS_FILTER_CONFIG
    
#ifdef  __cplusplus
extern "C" {
#endif

#define MM_VERSION_INFO "v0.1 12/19/13 21:29:22"
struct ais_server_config_s{
    struct {
         char* name;
         int port;
         int max_connections;
    } ais_tcp_server;
    int admin_http_port;
    struct {
         double y2;
         double y1;
         double x1;
         double x2;
    } geoserver;
    int ais_udp_in_port;
};
int ais_server_config_load(struct ais_server_config_s*,const char*);
void ais_server_config_close(struct ais_server_config_s*);
int ais_server_config_default(const char* f_);
      
    
#ifdef  __cplusplus
}
#endif

#endif /* __AIS_FILTER_CONFIG */ 
