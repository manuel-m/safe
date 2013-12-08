#ifndef __MSS_FILTER_CONFIG
#define __MSS_FILTER_CONFIG
    
#ifdef  __cplusplus
extern "C" {
#endif

struct mss_filter_config_s{
    int admin_http_port;
    int ais_udp_in_port;
    struct {
         double y2;
         double y1;
         double x1;
         double x2;
    } geofilter;
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
