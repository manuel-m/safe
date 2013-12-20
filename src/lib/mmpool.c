#include <stdlib.h>
#include <string.h>

#include "mmpool.h"

mmpool_t* mmpool_new(int max_, size_t item_size_, void* userdata_) {

    mmpool_t* pool = (mmpool_t*) calloc(1, sizeof (mmpool_t));
    if (NULL == pool) return NULL;
    pool->m_userdata = userdata_;
    pool->m_max = max_;
    pool->m_item_size = item_size_;
    pool->items = (mmpool_item_t**) calloc(max_, sizeof (mmpool_item_t*));
    pool->m_taken_total = 0;
    if (NULL == pool->items) {
        free(pool);
        return NULL;
    }
    return pool;
}

mmpool_item_t* mmpool_take(mmpool_t* pool_) {

    /* failed: full saturation */
    if (pool_->m_taken_len == pool_->m_max) return NULL;
    
    /* current alloc saturation, need extra alloc */
    if( pool_->m_taken_len == pool_->m_alloc_len){
        mmpool_item_t* item = (mmpool_item_t*)calloc(1,sizeof(mmpool_item_t));
        item->m_p = calloc(1, pool_->m_item_size);
        pool_->items[pool_->m_alloc_len] = item;
        ++(pool_->m_alloc_len);
        ++(pool_->m_taken_len);
        item->m_parent = pool_;
        item->m_state = 1;
        ++(pool_->m_taken_total);
        return item;
    }

    /* use a previously allocated but not used */    
    int i;
    for(i=0;i<=pool_->m_taken_len;i++){
        mmpool_item_t* item = pool_->items[i];
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
    int i;
    for(i=0;i<=pool_->m_alloc_len;i++){
        mmpool_item_t* item = pool_->items[i];
        if(item){
            free(item->m_p);
            free(item);
        }
    }    
    free(pool_);
}

void mmpool_iter_init(mmpool_iter_t* iter_, mmpool_t* pool_){
    iter_->m_pool = pool_;
    iter_->m_index = 0;
}

void* mmpool_iter_next(mmpool_iter_t* iter_){
    const mmpool_t* pool = iter_->m_pool;
    while(iter_->m_index < pool->m_alloc_len){
        mmpool_item_t* item = pool->items[iter_->m_index];
        ++(iter_->m_index);
        if(item->m_state) return item->m_p; /* only if valid */
    }
    return NULL;
}

void* mmpool_find(mmpool_finder_t* finder_, void* right_) {

    const mmpool_t* pool = finder_->m_pool;
    while (finder_->m_index < pool->m_alloc_len) {
        mmpool_item_t* item = pool->items[finder_->m_index];
        void *p = item->m_p;
        ++(finder_->m_index);
        if (item->m_state && (0 == finder_->m_cmp_cb(p, right_))) return p;
    }
    return NULL;
}