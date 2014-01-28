#include <stdlib.h>
#include <string.h>
#include "sad.h"

static char* current_string = NULL;

static int on_ais_decoded(struct sad_filter_s * f_){
    
    struct ais_t * ais = &f_->ais;

    if (3u < ais->type) return 0;

    const double lat = (double) ais->type1.lat * AIS_LATLON_DIV_INV;
    const double lon = (double) ais->type1.lon * AIS_LATLON_DIV_INV;

    /* mediterranean filter */
    if (lon > 0.0 && lon < 24.0 && lat < 44.0 && lat > 31.0)
       goto found;
    return 0;
    
found:;
    printf("%s",current_string);
    return 0;
    
}

#define MMUSAGE                              \
do {                                         \
  printf("%s : path/to/aivdm.nmea\n",argv[0]);     \
}                                            \
while(0);  

int main(int argc, char **argv) {
    if (2 != argc) {
      MMUSAGE;
      return -1;
    }
    sad_filter_t filter;
    if (sad_filter_init(&filter, on_ais_decoded, NULL, NULL)){
      MMUSAGE
      return -1;
    }
    
    FILE* f = fopen(argv[1], "r");
    if (NULL == f)return -1;
    struct gps_packet_t pck;

    while (fgets((char*) pck.inbuffer, sizeof (pck.inbuffer), f) != NULL) {
        current_string = (char*) pck.inbuffer;
        sad_decode_multiline(&filter, (char*) (pck.inbuffer + 14), (strlen((char*) pck.inbuffer))-14);
    }
    fclose(f);
    
    
    return 0;
}
