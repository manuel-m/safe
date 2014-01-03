/* 
 * pool buffer
 */



#ifndef __MMPOOL
#define	__MMPOOL

#ifdef	__cplusplus
extern "C" {
#endif

#include <stddef.h>    
    
    struct mmpool_s;
    
    typedef struct mmpool_item_s {
        unsigned m_state; /* 0:invalid 1:valid */
        struct mmpool_s* m_parent;
        void* m_p; /* internal data */
    } mmpool_item_t;

    typedef struct mmpool_s {
        unsigned m_alloc_max;
        unsigned m_alloc_step;
        unsigned m_alloc_len;
        unsigned m_taken_len;
        size_t m_item_size;
        void* m_userdata; 
        long long m_taken_total;
        mmpool_item_t* items;
    } mmpool_t;

    mmpool_t* mmpool_new(unsigned min_, unsigned max_, unsigned step_, 
                         size_t item_size_,void* m_userdata);
    
#define mmpool_easy_new(MAX,ITEMSIZE, USERDATA) mmpool_new( MAX, MAX, 1, ITEMSIZE, USERDATA)

#define mmpool_taken_len(PPOOL) PPOOL->m_taken_len 

    mmpool_item_t* mmpool_take(mmpool_t* pool_);
    void mmpool_giveback(mmpool_item_t* item_);
    void mmpool_free(mmpool_t* pool_);
    
    typedef struct mmpool_iter_s {
        unsigned m_index;
        mmpool_t* m_pool; 
    } mmpool_iter_t;

#define mmpool_iter_init(MMITER,MMPOOL)                                        \
do {                                                                           \
  MMITER.m_pool = (MMPOOL);                                                    \
  MMITER.m_index = 0;                                                          \
} while(0)
    
void* mmpool_iter_next(mmpool_iter_t*);


/**
 * compare callback, return 0 if equal
 */
typedef int (*mmcmp_cb)(void* left_, void* right_);

    typedef struct mmpool_finder_s {
        unsigned m_index;
        mmpool_t* m_pool;
        mmcmp_cb m_cmp_cb;
    } mmpool_finder_t;


#define mmpool_finder_init(MMFINDER,MMPOOL,CB)                                 \
do {                                                                           \
  MMFINDER.m_pool = (MMPOOL);                                                  \
  MMFINDER.m_index = 0;                                                        \
  MMFINDER.m_cmp_cb = CB;                                                      \
} while(0)    
    
mmpool_item_t* mmpool_find(mmpool_finder_t* finder_, void* right_);
    

#ifdef	__cplusplus
}
#endif

#endif	/* __MMPOOL */

