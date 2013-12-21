#include <stdlib.h>
#include <string.h>
#include "sad.h"

static int on_ais_decoded(struct sad_filter_s * filter_){
    
    printf("[ OK ] %08" PRIu64 " type:%02d mmsi:%09u  %s", 
            filter_->sentences, 
            filter_->ais.type, 
            filter_->ais.mmsi,             
            filter_->sentence->start);
    return 0;
}

#define MMUSAGE                              \
do {                                         \
  printf("%s : path/to/aivdm.txt\n",argv[0]);     \
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
    sad_decode_file(&filter, argv[1]);

    char* stats_str;
    sad_stats_string(&stats_str, &filter);

    if (stats_str) {
        printf("%s\n", stats_str);
        free(stats_str);
    }
    return 0;
}
