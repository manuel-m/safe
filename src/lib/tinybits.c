#include <stdio.h>
#include <limits.h>

#include "tinybits.h"

void tb_dump_unsigned_char(char* dest_, unsigned char n_) {
    
    unsigned i;
    for (i = sizeof (n_) * CHAR_BIT; 0u < i; i--) {
        *dest_ = '0' + ((n_ >> (i - 1)) & 1);
        ;
        ++dest_;
    }
}

void tb_dump_unsigned_char_array(char* dest_, unsigned char *data_,
        unsigned size_, char sep_) {
    
    unsigned j;
    unsigned char* p = data_;
    
    for (j = 0; size_ > j; j++) {
        unsigned i;
        for (i = sizeof (unsigned char) * CHAR_BIT; 0u < i; i--) {
            *dest_ = '0' + (((*p) >> (i - 1)) & 1);
            ++dest_;
        }
        *dest_ = sep_;
        ++dest_;
        ++p;
    }
}

//void tb_wbits_unsigned(unsigned char* dest_, unsigned bitsoffset_,
//        unsigned width_, unsigned n_) {
//    
//    unsigned octet_start = bitsoffset_ / CHAR_BIT;
//    
//    for (i = start / CHAR_BIT;
//    
//    
//
//
//
//}