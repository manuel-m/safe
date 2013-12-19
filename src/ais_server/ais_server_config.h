#ifndef __AIS_FILTER_CONFIG
#define __AIS_FILTER_CONFIG
    
#ifdef  __cplusplus
extern "C" {
#endif

#define MM_VERSION_INFO "v0.1 12/19/13 18:40:48"
struct ais_server_config_s{
    int ais_udp_in_port;
    int admin_http_port;
    struct {
         int port;
         int max_connections;
         char* name;
    } ais_tcp_server;
    struct {
         double x1;
         double x2;
         double y2;
         double y1;
    } geoserver;
    struct {
     int n;
     char** items ;
    } ais_out_udp;
};
int ais_server_config_load(struct ais_server_config_s*,const char*);
void ais_server_config_close(struct ais_server_config_s*);
int ais_server_config_default(const char* f_);
      
    
#ifdef  __cplusplus
}
#endif

#endif /* __AIS_FILTER_CONFIG */ 
