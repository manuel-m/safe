#ifndef MMCB_H
#define	MMCB_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct 
{
    void *buffer;
    unsigned index;
    unsigned capacity;
    unsigned capacity_flag;
    unsigned count;
    unsigned pow2_sz;
} cb_t;

typedef struct 
{
    cb_t* cb;
    unsigned index;
} cb_iter_t;


int cb_init(cb_t *cb_, unsigned pow2_capacity_, unsigned pow2_sz_);

void cb_iter_init(cb_iter_t *iter_, cb_t *cb_);
void* cb_iter_next(cb_iter_t *iter_);

void cb_free(cb_t *cb_);
void cb_push(cb_t *cb_, const void *item_,unsigned len_ );



#ifdef	__cplusplus
}
#endif

#endif	/* MMCB_H */

