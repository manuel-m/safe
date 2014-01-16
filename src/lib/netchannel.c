#include <string.h>

#include "netchannel.h"

int nc_in_channel_type_id( const char* str_ ){
    if(!strcmp(str_,NC_UDPSRV)) return NC_IN_UDPSRV; 
    if(!strcmp(str_,NC_TCPSRV)) return NC_IN_TCPSRV;
    if(!strcmp(str_,NC_TCPCLI)) return NC_IN_TCPCLI;
    return NC_IN_INVALID;
}

int nc_out_channel_ts_id( const char* str_ ){
    if(!strcmp(str_,NC_TIMESTAMP_NONE)) return NC_TS_NONE; 
    if(!strcmp(str_,NC_TIMESTAMP_UNSIGNED)) return NC_TS_UNSIGNED; 
    if(!strcmp(str_,NC_TIMESTAMP_HEX)) return NC_TS_HEX;
    return NC_TS_INVALID;
}

