#include<stdio.h> 
#include<string.h>
#include<stdlib.h>
#include<time.h>

#include<arpa/inet.h>
#include<sys/socket.h>
#include <unistd.h>

#include "mmtrace.h"
#include "sad.h"

#define MM_TARGET_ADDR "127.0.0.1"
#define MM_BUFLEN 512 
#define PORT 9990

#define MM_TIMESTAMP_CSV_OFFSET 14
#define MM_MAX_NMEA_LEN 127

static char current_nmea[MM_MAX_NMEA_LEN];
unsigned nmea_without_ts_n = 0;
static const char* nmea_without_ts = (char*) (current_nmea + MM_TIMESTAMP_CSV_OFFSET);
static int udp_client_socket;
static struct sockaddr_in si_other;
static socklen_t slen = sizeof (si_other);


int on_ais_decoded(struct sad_filter_s * f_) {

    int r = 0;
    struct ais_t * ais = &f_->ais;
    if (3u < ais->type) return 0;

    const double lat = (double) ais->type1.lat * AIS_LATLON_DIV_INV;
    const double lon = (double) ais->type1.lon * AIS_LATLON_DIV_INV;

    /* mediterranean filter */
    if (lon > 0.0 && lon < 24.0 && lat < 44.0 && lat > 31.0)
        goto found;
    return 0;

found:
    ;
    if (0 > sendto(udp_client_socket, nmea_without_ts, nmea_without_ts_n, 0,
            (struct sockaddr *) &si_other, slen)) MM_GERR("sendto()");

end:
    return r;

err:
    r = -1;
    goto end;

}

int main(int argc, char **argv) {
    int res = 0;

    if (2 != argc) MM_GERR("%s : path/to/file.nmea\n", argv[0]);

    sad_filter_t filter;
    if (sad_filter_init(&filter, on_ais_decoded, NULL, NULL)) MM_GERR("internal");

    FILE* nmea_file = fopen(argv[1], "r");

    if (0 > (udp_client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))) MM_GERR("socket");

    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    if (0 == inet_aton(MM_TARGET_ADDR, &si_other.sin_addr))MM_GERR("inet_aton() failed");

    memset(current_nmea, 0, sizeof (current_nmea));
    
    unsigned ts = 0;
    unsigned read_ts = 0;
    unsigned offset_ts = 0;
    unsigned req_sleep = 0;
    
    /* only 10 first digit for timestamp ...*/
    char ts_string[10 + 1];
    ts_string[10]='\0';
    
    while (fgets((char*) current_nmea, sizeof (current_nmea), nmea_file) != NULL) {
        
        ts = (unsigned)time(NULL);
        memcpy(ts_string,current_nmea, 10);
        read_ts = (unsigned)atoi(ts_string);
        
        if(0u == offset_ts){
            offset_ts = ts - read_ts;
        }
        req_sleep = read_ts + offset_ts - ts;
//        MM_INFO("sleep:%u",req_sleep);
        if(req_sleep > 0) sleep (req_sleep);
        
        nmea_without_ts_n = strlen(current_nmea) - MM_TIMESTAMP_CSV_OFFSET;
        sad_decode_multiline(&filter, nmea_without_ts, nmea_without_ts_n);
    }

end:
    close(udp_client_socket);
    fclose(nmea_file);
    return res;

err:
    res = -1;
    goto end;
}