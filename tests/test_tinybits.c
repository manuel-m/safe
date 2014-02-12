#include <stdio.h>
#include <string.h>

#include "mmtest.h"
#include "tinybits.h"

MMTEST_DECL



int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    char buffer[2048];

    // 0
    memset(buffer, 0, sizeof (buffer));
    tb_dump_unsigned_char(buffer, 0x0);
    MMTEST((0 == strcmp("00000000", buffer)));

    // 255
    memset(buffer, 0, sizeof (buffer));
    tb_dump_unsigned_char(buffer, 0xFF);
    MMTEST((0 == strcmp("11111111", buffer)));

    // 3
    memset(buffer, 0, sizeof (buffer));
    tb_dump_unsigned_char(buffer, 3);
    MMTEST((0 == strcmp("00000011", buffer)));

    // unsigned int
    {
        unsigned ui = 211169260u;
        
        memset(buffer, 0, sizeof (buffer));
        tb_dump_unsigned_char(buffer, ui);
        MMTEST((0 == strcmp("11101100", buffer)));

        memset(buffer, 0, sizeof (buffer));
        tb_dump_unsigned_char_array(buffer, (unsigned char*)&ui, sizeof (unsigned), ' ');
        MMTEST((0 == strcmp("11101100 00101111 10010110 00001100 ", buffer)));

    }
    // 4,9,64
    {
        unsigned char testset[] = {4u, 9u, 64u};
        memset(buffer, 0, sizeof (buffer));
        tb_dump_unsigned_char_array(buffer, testset, sizeof (testset) /
                sizeof (unsigned char), ' ');
        MMTEST((0 == strcmp("00000100 00001001 01000000 ", buffer)));
    }

    {
        unsigned dest=0;
        
        tb_wbits_unsigned((unsigned char*)&dest, 3, 32, 7);
        
        memset(buffer, 0, sizeof (buffer));
        
        tb_dump_unsigned_char_array(buffer, (unsigned char*)&dest, 4, ' ');
        
        printf("%s\n",buffer);
        
        

    }


    MMTEST_END
}

