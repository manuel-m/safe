#ifndef __MSS_FILTER_CONFIG
#define __MSS_FILTER_CONFIG
    
#ifdef  __cplusplus
extern "C" {
#endif

struct mss_filter_config_s{
    struct {
         int max_connections;
         char* name;
         int port;
    } ais_tcp_server;
    int admin_http_port;
    struct {
         double y1;
         double y2;
         double x2;
         double x1;
    } geofilter;
    int ais_udp_in_port;
    struct {
     int n;
     char** items ;
    } ais_out_udp;
};
int mss_filter_config_load(struct mss_filter_config_s*,const char*);
void mss_filter_config_close(struct mss_filter_config_s*);
int mss_filter_config_default(const char* f_);
      
    
#ifdef  __cplusplus
}
#endif

#endif /* __MSS_FILTER_CONFIG */ 
