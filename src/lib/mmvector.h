#ifndef MMVECTOR_H
#define MMVECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#define MM_DECL_VECTOR(TYPE)                                                   \
struct {                                                                       \
  unsigned n;                                                                  \
  TYPE * items;}                                                               
  
#define MM_FREE_VECTOR(NAME)                                                   \
do{                                                                            \
  if((NAME).items) free((NAME).items);                                         \
} while(0);

#define MM_ALLOC_VECTOR(NAME,TYPE,LEN)                                         \
do{                                                                            \
  (NAME).n = (LEN);                                                            \
  (NAME).items = calloc((LEN),sizeof(TYPE));                                   \
} while(0);


#ifdef __cplusplus
}
#endif

#endif /* MMVECTOR_H */