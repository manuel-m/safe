#ifndef __NETCHANNEL_H
#define	__NETCHANNEL_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct {
    const char* name;
    int  port;
    int max_connections;
    int ts_id;
    const char* timestamp;
} channel_out_values_t;

typedef struct {
    int type_id;
    const char* type;
    const char* name;
    const char* addr;
    int port;
} channel_in_values_t;

typedef struct {
    const char* addr;
    int port;
} out_ais_udp_t;

#define NC_UDPSRV "udp_server"
#define NC_TCPSRV "tcp_server"
#define NC_TCPCLI "tcp_client"

#define NC_IN_INVALID -1
#define NC_IN_UDPSRV 0
#define NC_IN_TCPSRV 1
#define NC_IN_TCPCLI 2

#define NC_TS_INVALID -1
#define NC_TS_NONE     0
#define NC_TS_UNSIGNED 1
#define NC_TS_HEX      2

#define NC_TIMESTAMP_NONE     "none"
#define NC_TIMESTAMP_UNSIGNED "unsigned"
#define NC_TIMESTAMP_HEX      "hex"

int nc_in_channel_type_id( const char* str_ );
int nc_out_channel_ts_id( const char* str_ );

   



#ifdef	__cplusplus
}
#endif

#endif	/* __NETCHANNEL_H */

