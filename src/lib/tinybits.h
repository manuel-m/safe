#ifndef __TINYBITS
#define __TINYBITS

#ifdef  __cplusplus
extern "C" {
#endif

void tb_dump_unsigned_char(char* dest_,unsigned char data_);
void tb_dump_unsigned_char_array(char* dest_,unsigned char *data_, unsigned size_, char sep_);


#ifdef  __cplusplus
}
#endif

#endif  /* __TINYBITS */

