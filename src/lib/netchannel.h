#ifndef __NETCHANNEL_H
#define	__NETCHANNEL_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct {
    const char* name;
    int  port;
    int max_connections;
} channel_out_values_t;

typedef struct {
    int type_id;
    const char* type;
    const char* name;
    const char* addr;
    int port;
} channel_in_values_t;

#define NC_UDPSRV "udp_server"
#define NC_TCPSRV "tcp_server"
#define NC_TCPCLI "tcp_client"

#define NC_IN_INVALID -1
#define NC_IN_UDPSRV 0
#define NC_IN_TCPSRV 1
#define NC_IN_TCPCLI 2

int nc_int_channel_status( const char* str_ );

   



#ifdef	__cplusplus
}
#endif

#endif	/* __NETCHANNEL_H */

