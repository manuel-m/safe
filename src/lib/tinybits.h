#ifndef __TINYBITS
#define __TINYBITS

#ifdef  __cplusplus
extern "C" {
#endif

#define tb_tst(xxVARxx,xxBITxx) (xxVARxx)&(1<<(xxBITxx))
#define tb_set(xxVARxx,xxBITxx) (xxVARxx)|= 1<<(xxBITxx)
#define tb_clr(xxVARxx,xxBITxx) (xxVARxx)&= ~(1<<(xxBITxx))
    
    void tb_dump_binary_little(unsigned s_, void* p_, char sep_);
    void tb_dump_binary_big(unsigned s_, void* p_, char sep_);
//    void tb_dump_binary_big(unsigned s, void* p);

    void tb_dump_unsigned_char(char* dest_, unsigned char data_);
    void tb_dump_unsigned_char_array(char* dest_, unsigned char *data_,
            unsigned size_, char sep_);

    void tb_wbits_unsigned(unsigned char* dest_, unsigned bitsoffset_,
            unsigned width_, unsigned src_);


#ifdef  __cplusplus
}
#endif

#endif  /* __TINYBITS */

