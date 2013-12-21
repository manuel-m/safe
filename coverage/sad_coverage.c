#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "sad.h"


static int on_ais_decoded(struct sad_filter_s * filter_){
    (void)filter_;
    return 0;
}

int main(void) {

    char cwd[2048];
    sad_filter_t filter;

    if (getcwd(cwd, sizeof (cwd)) != NULL)
    {
        char buf[4096];
        
        if (sad_filter_init(&filter, on_ais_decoded, NULL,NULL)) return -1;     
        sprintf(buf, "%s/../../resources/data/test5k.aivdm", cwd);
	printf("[INFO] using for coverage test %s\n",buf);
        sad_decode_file(&filter, buf);
        return 0;
    }
    
    char* stats_str;
    sad_stats_string(&stats_str, &filter);

    if (stats_str) {
        printf("%s\n", stats_str);
        free(stats_str);
    }        
    
    return 1;
}


