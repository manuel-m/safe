#include <stdio.h>
#include <limits.h>

#include "tinybits.h"

void tb_dump_binary_little(unsigned s_, void* p_, char sep_) {
    int i, j;
    for (i = s_ - 1; i >= 0; i--) {
        for (j = 7; j >= 0; j--)
            printf("%u", (*((unsigned char*) p_ + i)&(1 << j)) >> j);
        if (sep_)printf("%c", sep_);
    }
}

void tb_dump_binary_big(unsigned s_, void* p_, char sep_)
{
    unsigned char *b = (unsigned char*) p_;
    unsigned char byte;
    int i, j;

    for (i=0;i<s_;i++)
    {
        for (j=7;j>=0;j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
        if(sep_)printf("%c",sep_);
    }
}


/*
void tb_dump_binary(unsigned s, void* p) {
    int i, j;
    for (i = 0; i < s; i++)
        for (j = 7; j >= 0; j--)
            printf("%u", (*((unsigned char*) p + i)&(1 << j)) >> j);
    puts("");
}
*/

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
        if(sep_){
        *dest_ = sep_;
        ++dest_;
        }
        ++p;
    }
}

void tb_wbits_unsigned(unsigned char* dest_, unsigned  bitsoffset_,
        unsigned width_, unsigned src_) {
    
    unsigned i;
    (void)bitsoffset_;
        
    for (i = 0 ; i < width_ ; i++){
        
        unsigned char* c  = dest_  + ((i + 0) / CHAR_BIT);
        
//        printf("%p %p \n", dest_, c);
        
        unsigned  b  =  i % CHAR_BIT;
        
//        *p  |= src_ & (1 << (width_ - i - 1));
        if (src_ & (1 << (width_ - i - 1))){
          *c  |= 1 << b;
        }
        
        /*
        unsigned char* c  = dest_  + (3 - ((bitsoffset_+i)/ CHAR_BIT));
        unsigned  b  =  (bitsoffset_+i) % CHAR_BIT;

           if (src_&(1 << i)){
               *c |= (1<<b); 
           }
        */
    } 
    
    



}