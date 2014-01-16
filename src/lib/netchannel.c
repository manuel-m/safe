#include <string.h>

#include "netchannel.h"

int nc_int_channel_status( const char* str_ ){
    if(!strcmp(str_,NC_UDPSRV)) return NC_IN_UDPSRV; 
    if(!strcmp(str_,NC_TCPSRV)) return NC_IN_TCPSRV;
    if(!strcmp(str_,NC_TCPCLI)) return NC_IN_TCPCLI;
    return NC_IN_INVALID;
}
