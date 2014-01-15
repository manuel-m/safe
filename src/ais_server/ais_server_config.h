#ifndef __AIS_SERVER_CONFIG
#define __AIS_SERVER_CONFIG
    
#ifdef  __cplusplus
extern "C" {
#endif

#define MM_VERSION_INFO "v0.1 01/15/14 16:52:34"
struct ais_server_config_s{
    struct {
         char* name;
         int port;
         int max_connections;
    } ais_tcp_server;
    int step_ships;
    int ais_udp_in_port;
    int admin_http_port;
    int min_ships;
    int max_ships;
};
int ais_server_config_load(struct ais_server_config_s*,const char*);
void ais_server_config_close(struct ais_server_config_s*);
int ais_server_config_default(const char* f_);
      
    
#ifdef  __cplusplus
}
#endif

#endif /* __AIS_SERVER_CONFIG */ 
