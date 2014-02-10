#include<stdio.h> 
#include<string.h>
#include<stdlib.h>

#include "mmtrace.h"


int main(int argc, char **argv) {
    int res = 0;
    
    unsigned idx=0; 
    unsigned sampling_ratio=0; 
    unsigned window_size=0;
    unsigned window_size_current=0;
    static char current_line[2048];
    FILE* input_file = 0;
    
    
    if (4 != argc) MM_GERR("usage\n\t %s path/to/file sampling_ratio window_size\n", argv[0]);
    
    sampling_ratio = (unsigned) atoi(argv[2]);
    window_size = (unsigned) atoi(argv[3]);
    
    if( !sampling_ratio ) MM_GERR("invalid sampling_ratio %s", argv[2]);
    if( sampling_ratio <= window_size ) MM_GERR("%s >= %s", argv[2], argv[3]);

    input_file = fopen(argv[1], "r");

    while (fgets((char*) current_line, sizeof (current_line), input_file) != NULL) {
        if(!idx) window_size_current = window_size;
        if(window_size_current){
          printf("%s",current_line);          
          --window_size_current;
        }          
        ++idx;
        idx %= sampling_ratio;
        
    }

end:
    if(input_file)fclose(input_file);
    return res;

err:
    res = -1;
    goto end;
}