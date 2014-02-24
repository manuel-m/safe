#include<stdio.h> 
#include<string.h>
#include<stdlib.h>

#include <unistd.h>

#include "mmtrace.h"
#include "sad.h"


#define MM_TIMESTAMP_CSV_OFFSET 14
#define MM_MAX_NMEA_LEN 127


static FILE* file1 = 0;
static FILE* file2 = 0;

static char nmea1[MM_MAX_NMEA_LEN];
static char nmea2[MM_MAX_NMEA_LEN];

int main(int argc, char **argv) {

    int res = 0;
    if (3 != argc) MM_GERR("%s path/to/file1.nmea path/to/file2.nmea\n", argv[0]);

    file1 = fopen(argv[1], "r");
    file2 = fopen(argv[2], "r");


    memset(nmea1, 0, sizeof (nmea1));
    memset(nmea2, 0, sizeof (nmea2));
    

    /* only 10 first digits for timestamp ...*/
    char ts1_string[13 + 1];
    char ts2_string[13 + 1];
    char ts2_modstring[13 + 1];

    ts1_string[13] = '\0';
    ts2_string[13] = '\0';
    ts2_modstring[13] = '\0';

    if (NULL == fgets((char*) nmea1, sizeof (nmea1), file1)) goto err;
    fclose(file1);
    
    if (NULL == fgets((char*) nmea2, sizeof (nmea2), file2)) goto err;
    rewind(file2);

    memcpy(ts1_string, nmea1, 13);
    memcpy(ts2_string, nmea2, 13);

    const long ts1 = atol(ts1_string);
    long ts2 = (unsigned) atol(ts2_string);

    if (0 == ts1) goto err;
    if (0 == ts2) goto err;

    long offset_ts = ts1 - ts2;
    
    while(fgets((char*) nmea2, sizeof (nmea2), file2) != NULL){
        
        memcpy(ts2_string, nmea2, 13);
        long ts2 = (unsigned) atol(ts2_string);
        ts2 += offset_ts;
        
        sprintf(ts2_modstring,"%013ld",ts2);
        memcpy(nmea2, ts2_modstring, 13);
        printf("%s",nmea2);
    }




end:
    if (file2)fclose(file2);
    return res;

err:
    res = -1;
    goto end;
}
