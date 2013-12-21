#ifndef __AIS_SERVER_CONFIG
#define __AIS_SERVER_CONFIG
    
#ifdef  __cplusplus
extern "C" {
#endif

#define MM_VERSION_INFO "v0.1 12/22/13 00:44:27"
struct ais_server_config_s{
    int ais_udp_in_port;
    struct {
         int port;
         int max_connections;
         char* name;
    } ais_tcp_server;
    int max_ships;
    int admin_http_port;
};
int ais_server_config_load(struct ais_server_config_s*,const char*);
void ais_server_config_close(struct ais_server_config_s*);
int ais_server_config_default(const char* f_);
      
    
#ifdef  __cplusplus
}
#endif

#endif /* __AIS_SERVER_CONFIG */ 
