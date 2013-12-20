#ifndef __AIS_SERVER_CONFIG
#define __AIS_SERVER_CONFIG
    
#ifdef  __cplusplus
extern "C" {
#endif

#define MM_VERSION_INFO "v0.1 12/20/13 00:20:21"
struct ais_server_config_s{
    int max_ships;
    int ais_udp_in_port;
    int admin_http_port;
    struct {
         int port;
         int max_connections;
         char* name;
    } ais_tcp_server;
};
int ais_server_config_load(struct ais_server_config_s*,const char*);
void ais_server_config_close(struct ais_server_config_s*);
int ais_server_config_default(const char* f_);
      
    
#ifdef  __cplusplus
}
#endif

#endif /* __AIS_SERVER_CONFIG */ 
