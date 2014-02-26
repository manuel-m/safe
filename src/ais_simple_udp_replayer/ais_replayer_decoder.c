#include<stdio.h> 
#include<string.h>
#include<stdlib.h>

#include <unistd.h>

#include "mmtrace.h"
#include "sad.h"

#define MM_MAXVESSELS 8192

#define MM_TIMESTAMP_CSV_OFFSET 14
#define MM_MAX_NMEA_LEN 127

static char current_nmea[MM_MAX_NMEA_LEN];
unsigned nmea_without_ts_n = 0;
static const char* nmea_without_ts = (char*) (current_nmea + MM_TIMESTAMP_CSV_OFFSET);

static unsigned read_ts = 0;


static FILE* nmea_file = 0;

struct mmsi_ts {
    unsigned mmsi;
    unsigned last_ts;
};

static sad_filter_t filter;

int on_ais_decoded(struct sad_filter_s * f_) {

    int r = 0;
    const struct ais_t * ais = &f_->ais;

    // no we only want 1,2,3 message types
    if (3u < ais->type) goto dismiss;

    // we are here to output something
    {
        const double lat = (double) ais->type1.lat * AIS_LATLON_DIV_INV;
        const double lon = (double) ais->type1.lon * AIS_LATLON_DIV_INV;
        printf("%u,%u,%f,%f\n", read_ts,ais->mmsi,lat,lon);
    }
    
dismiss:
    return r;
}

int main(int argc, char **argv) {

    int res = 0;
    
    if (2 != argc) MM_GERR("%s path/to/file.nmea \n", argv[0]);


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
    if (nmea_file)fclose(nmea_file);
    return res;

err:
    res = -1;
    goto end;
}
