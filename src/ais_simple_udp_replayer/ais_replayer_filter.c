#include<stdio.h> 
#include<string.h>
#include<stdlib.h>

#include <unistd.h>

#include "mmtrace.h"
#include "sad.h"

#define MM_MAXVESSELS 4096

#define MM_TIMESTAMP_CSV_OFFSET 14
#define MM_MAX_NMEA_LEN 127

static char current_nmea[MM_MAX_NMEA_LEN];
unsigned nmea_without_ts_n = 0;
static const char* nmea_without_ts = (char*) (current_nmea + MM_TIMESTAMP_CSV_OFFSET);

static unsigned read_ts = 0;
static unsigned sampling = 0;
static unsigned vessels_count = 0;

static unsigned banned_mmsi_size = 0;
static unsigned* banned_mmsi_array = NULL;

static FILE* nmea_file = 0;

struct mmsi_ts {
  unsigned mmsi;
  unsigned last_ts;  
};

static struct mmsi_ts mmsi_buffer[MM_MAXVESSELS];

static sad_filter_t filter;

int on_ais_decoded(struct sad_filter_s * f_) {

    int r = 0;
    const struct ais_t * ais = &f_->ais;

    // check if mmsi inside banned list
    if (banned_mmsi_size) {
        int i;
        for (i = 0; i < banned_mmsi_size; i++) {
            if( ais->mmsi == banned_mmsi_array[i]) goto dismiss;
        }
    }    
    
    // let static voyage pass
    if (5u == ais->type) goto accept;
    
    // no we only want 1,2,3 message types
    if (3u < ais->type) goto dismiss;
    
    /* mediterranean filter
       46.34693, -9.93164
       28.65203, 38.05664 
    */
    const double lat = (double) ais->type1.lat * AIS_LATLON_DIV_INV;
    const double lon = (double) ais->type1.lon * AIS_LATLON_DIV_INV;

    // yes it's hardcoded ...
    if (!(lon > -9.93 && lon < 38.0 && lat < 46.0 && lat > 30.0)) goto dismiss;
    
    // search into sampling buffer 
    int i;
    for(i=0;i<MM_MAXVESSELS;i++){
      if(ais->mmsi == mmsi_buffer[i].mmsi){
        if(read_ts - mmsi_buffer[i].last_ts >= sampling){
          mmsi_buffer[i].last_ts = read_ts;
          goto accept;
        } else goto dismiss; // dismiss
      }
    }
    
    // not found, add new vessel to sampling buffer
    if(vessels_count < MM_MAXVESSELS){
      mmsi_buffer[vessels_count].mmsi = ais->mmsi;
      mmsi_buffer[vessels_count].last_ts = read_ts;
      ++vessels_count;
      goto accept;
    }
    
    // new vessel saturation, it's an error
    goto err;
    
accept:
    printf("%s",current_nmea);

dismiss:
    return r;

err:
    r = -1;
    goto dismiss;

}

int main(int argc, char **argv) {
  
    int res = 0;
    memset(mmsi_buffer,0,sizeof(mmsi_buffer));

    if (3 > argc) MM_GERR("%s path/to/file.nmea sampling [mmsi list to exclude]\n", argv[0]);
           
    sampling = (unsigned) atoi(argv[2]);
    
    // we do have filtered mmsi
    {
        banned_mmsi_size = argc - 3;
        banned_mmsi_array = calloc(banned_mmsi_size, sizeof (unsigned));
        if (NULL == banned_mmsi_array) goto err;

        if (banned_mmsi_size) {
            int i;
            for (i = 0; i < banned_mmsi_size; i++) {
                const unsigned current = atoi(argv[i + 3]);
                
                // something went ..  wrong exit
                if (0 == current) goto err;
                banned_mmsi_array[i] = current;
            }
        }
    }
    

    
    if (sad_filter_init(&filter, on_ais_decoded, NULL, NULL)) MM_GERR("internal");

    nmea_file = fopen(argv[1], "r");

    memset(current_nmea, 0, sizeof (current_nmea));

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
    if(banned_mmsi_array)free(banned_mmsi_array);
    return res;

err:
    res = -1;
    goto end;
}
