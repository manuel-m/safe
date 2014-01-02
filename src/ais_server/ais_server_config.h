#ifndef __AIS_SERVER_CONFIG
#define __AIS_SERVER_CONFIG
    
#ifdef  __cplusplus
extern "C" {
#endif

#define MM_VERSION_INFO "v0.1 01/02/14 16:17:10"
struct ais_server_config_s{
    int max_ships;
    int step_ships;
    int ais_udp_in_port;
    struct {
         int port;
         char* name;
         int max_connections;
    } ais_tcp_server;
    int admin_http_port;
    int min_ships;
};
int ais_server_config_load(struct ais_server_config_s*,const char*);
void ais_server_config_close(struct ais_server_config_s*);
int ais_server_config_default(const char* f_);
      
    
#ifdef  __cplusplus
}
#endif

#endif /* __AIS_SERVER_CONFIG */ 
