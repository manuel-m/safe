#include<stdio.h> 
#include<string.h>
#include<stdlib.h>
#include<time.h>

#include<arpa/inet.h>
#include<sys/socket.h>
#include <unistd.h>

#include "mmtrace.h"
#include "sad.h"

#define MM_BUFLEN 512 
#define MM_MAXVESSELS 32

#define MM_TIMESTAMP_CSV_OFFSET 14
#define MM_MAX_NMEA_LEN 127

static char current_nmea[MM_MAX_NMEA_LEN];
unsigned nmea_without_ts_n = 0;
static const char* nmea_without_ts = (char*) (current_nmea + MM_TIMESTAMP_CSV_OFFSET);

static unsigned read_ts = 0;
static unsigned sampling = 0;
static unsigned vessels_count = 0;


struct mmsi_ts {
  unsigned mmsi;
  unsigned last_ts;  
};

static struct mmsi_ts mmsi_buffer[MM_MAXVESSELS];


int on_ais_decoded(struct sad_filter_s * f_) {

    int r = 0;
    struct ais_t * ais = &f_->ais;
    if (3u < ais->type) return 0;
    
    // search
    int i;
    for(i=0;i<MM_MAXVESSELS;i++){
      if(ais->mmsi == mmsi_buffer[i].mmsi){
        if(read_ts - mmsi_buffer[i].last_ts >= sampling){
          mmsi_buffer[i].last_ts = read_ts;
          goto pass;
        } else goto end; // dismiss
      }
    }
    
    // not found, add new vessel to buffer
    if(vessels_count < MM_MAXVESSELS){
      mmsi_buffer[vessels_count].mmsi = ais->mmsi;
      mmsi_buffer[vessels_count].last_ts = read_ts;
      ++vessels_count;
      goto pass;
    }
    
    // new vessel saturation, it's an error
    goto err;
    
pass:
    printf(current_nmea);

end:
    return r;

err:
    r = -1;
    goto end;

}

int main(int argc, char **argv) {
  
    int res = 0;
    memset(mmsi_buffer,0,sizeof(mmsi_buffer));

    if (3 != argc) MM_GERR("%s : path/to/file.nmea sampling\n", argv[0]);
           
    sampling = (unsigned) atoi(argv[2]);

    sad_filter_t filter;
    if (sad_filter_init(&filter, on_ais_decoded, NULL, NULL)) MM_GERR("internal");

    FILE* nmea_file = fopen(argv[1], "r");

    memset(current_nmea, 0, sizeof (current_nmea));

    unsigned ts = 0;
    
    unsigned offset_ts = 0;

    /* only 10 first digits for timestamp ...*/
    char ts_string[10 + 1];
    ts_string[10] = '\0';

    while (fgets((char*) current_nmea, sizeof (current_nmea), nmea_file) != NULL) {

        memcpy(ts_string, current_nmea, 10);
        read_ts = (unsigned) atoi(ts_string);

        nmea_without_ts_n = strlen(current_nmea) - MM_TIMESTAMP_CSV_OFFSET;
        sad_decode_multiline(&filter, nmea_without_ts, nmea_without_ts_n);
    }

end:
    if(nmea_file)fclose(nmea_file);
    return res;

err:
    res = -1;
    goto end;
}