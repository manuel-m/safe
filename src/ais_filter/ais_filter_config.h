#ifndef __MSS_FILTER_CONFIG
#define __MSS_FILTER_CONFIG
    
#ifdef  __cplusplus
extern "C" {
#endif

struct ais_filter_config_s{
    struct {
         double y2;
         double x2;
         double x1;
         double y1;
    } geofilter;
    int admin_http_port;
    struct {
         int port;
         char* name;
         int max_connections;
    } ais_tcp_server;
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

#endif /* __MSS_FILTER_CONFIG */ 
