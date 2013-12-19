#ifndef __AIS_FILTER_CONFIG
#define __AIS_FILTER_CONFIG
    
#ifdef  __cplusplus
extern "C" {
#endif

#define MM_VERSION_INFO "v0.1 12/19/13 17:26:23"
struct ais_filter_config_s{
    struct {
         double x1;
         double x2;
         double y2;
         double y1;
    } geofilter;
    int admin_http_port;
    int ais_udp_in_port;
    struct {
         int port;
         int max_connections;
         char* name;
    } ais_tcp_server;
    struct {
     int n;
     char** items ;
    } ais_out_udp;
};
int ais_filter_config_load(struct ais_filter_config_s*,const char*);
void ais_filter_config_close(struct ais_filter_config_s*);
int ais_filter_config_default(const char* f_);
      
    
#ifdef  __cplusplus
}
#endif

#endif /* __AIS_FILTER_CONFIG */ 
