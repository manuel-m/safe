#include <stdlib.h>
#include <string.h>

#include "mmpool.h"

mmpool_t* mmpool_new(unsigned min_, unsigned max_, unsigned step_, 
                     size_t item_size_, void* userdata_) {

    mmpool_t* pool = (mmpool_t*) calloc(1, sizeof (mmpool_t));
    if (NULL == pool) return NULL;
    pool->m_userdata = userdata_;
    pool->m_alloc_max = max_;
    pool->m_alloc_step = step_;
    pool->m_item_size = item_size_;
    
    pool->items = (mmpool_item_t*) calloc(min_, sizeof (mmpool_item_t));
    if (NULL == pool->items) goto err;
    
    unsigned i;
    mmpool_item_t* p = pool->items;
    for(i=0;i<min_;i++){
      p->m_p = calloc(1, item_size_);
      p->m_parent = pool;
      ++p;
    }
    
    pool->m_alloc_len = min_;
    pool->m_taken_total = 0;
    
    return pool;
    
err:

    if(pool && pool->items) free(pool->items);
    if(pool) free(pool);
    return NULL;
}

mmpool_item_t* mmpool_take(mmpool_t* pool_) {

    /* failed: full saturation */
    if (pool_->m_taken_len == pool_->m_alloc_max) return NULL;
    
    /* current alloc saturation, need extra alloc */
    if( pool_->m_taken_len == pool_->m_alloc_len){
      
    const unsigned previous_size = pool_->m_alloc_len;
    unsigned next_size = pool_->m_alloc_len + pool_->m_alloc_step;
    if(next_size > pool_->m_alloc_max) next_size = pool_->m_alloc_max;
    const unsigned size_increase = next_size - previous_size;
    
    mmpool_item_t* more_items = (mmpool_item_t*) realloc(pool_->items, next_size * sizeof (mmpool_item_t));
    
    if(NULL == more_items) {
      free(more_items);
      return NULL;
    }
    pool_->items = more_items;
    
    mmpool_item_t* p = &pool_->items[previous_size];
    unsigned i;
    for(i=0;i<size_increase;i++){
      p->m_p = calloc(1, pool_->m_item_size);
      p->m_parent = pool_;
      p->m_state = 0;
      ++p;
    }
     pool_->m_alloc_len+=size_increase;
     
     mmpool_item_t* item = &pool_->items[previous_size];
     ++(pool_->m_taken_len);
     item->m_state = 1;
     ++(pool_->m_taken_total);
     return item;
    }

    /* use a previously allocated but not used */    
    unsigned i;
    for(i=0;i<=pool_->m_taken_len;i++){
        mmpool_item_t* item = &pool_->items[i];
        if(0 == item->m_state){
            item->m_state = 1;
            ++(pool_->m_taken_len);
            ++(pool_->m_taken_total);
            return item;
        }
    }
    return NULL;
}

void mmpool_giveback(mmpool_item_t* item_) {
    --(item_->m_parent->m_taken_len);
    item_->m_state = 0;
}

void mmpool_free(mmpool_t* pool_) {
    if(NULL == pool_) return;
    unsigned i;
    for(i=0;i<pool_->m_alloc_len;i++){
        mmpool_item_t* item = &pool_->items[i];
        if(item){
            free(item->m_p);
        }
    }    
    free(pool_->items);
    free(pool_);
}



void* mmpool_iter_next(mmpool_iter_t* iter_){
    const mmpool_t* pool = iter_->m_pool;
    while(iter_->m_index < pool->m_alloc_len){
        mmpool_item_t* item = &pool->items[iter_->m_index];
        ++(iter_->m_index);
        if(item->m_state) return item->m_p; /* only if valid */
    }
    return NULL;
}

mmpool_item_t* mmpool_find(mmpool_finder_t* finder_, void* right_) {

    const mmpool_t* pool = finder_->m_pool;
    while (finder_->m_index < pool->m_alloc_len) {
        mmpool_item_t* item = &pool->items[finder_->m_index];
        void *p = item->m_p;
        ++(finder_->m_index);
        if (item->m_state && (0 == finder_->m_cmp_cb(p, right_))) return item;
    }
    return NULL;
}