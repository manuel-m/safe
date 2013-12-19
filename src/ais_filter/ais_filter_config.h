#ifndef __AIS_FILTER_CONFIG
#define __AIS_FILTER_CONFIG
    
#ifdef  __cplusplus
extern "C" {
#endif

#define MM_VERSION_INFO "v0.1 12/19/13 21:29:22"
struct ais_filter_config_s{
    struct {
         int port;
         char* name;
         int max_connections;
    } ais_tcp_server;
    int admin_http_port;
    struct {
         double x2;
         double x1;
         double y1;
         double y2;
    } geofilter;
    int ais_udp_in_port;
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
