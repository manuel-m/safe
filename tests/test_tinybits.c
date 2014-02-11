#include <stdio.h>
#include <string.h>

#include "mmtest.h"
#include "tinybits.h"

MMTEST_DECL



int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    char buffer[2048];memset(buffer,0,sizeof(buffer));
    
    // 0
    tb_dump_unsigned_char(buffer, 0x0);
    MMTEST((0 == strcmp("00000000", buffer)));
    
    // 255
    tb_dump_unsigned_char(buffer, 0xFF);
    MMTEST((0 == strcmp("11111111", buffer)));

    // 3
    tb_dump_unsigned_char(buffer, 3);
    MMTEST((0 == strcmp("00000011", buffer)));
    
    // 4,9,64
    unsigned char testset[] = { 4u,9u,64u };
    tb_dump_unsigned_char_array(buffer,testset, sizeof(testset)/sizeof(unsigned char), ' ');
    MMTEST((0 == strcmp("00000100 00001001 01000000 ", buffer)));
    
    

    
    MMTEST_END
}

