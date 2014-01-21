#include <string.h>
#include <stdlib.h>

#include "mmcb.h"

int cb_init(cb_t *cb_, unsigned pow2_capacity_, unsigned pow2_sz_) {
    memset(cb_, 0, sizeof (cb_t));
    cb_->capacity = (1 << pow2_capacity_);
    cb_->pow2_sz = pow2_sz_;
    cb_->buffer = calloc(cb_->capacity, (1 << pow2_sz_));
    if (cb_->buffer == NULL) return -1;
    cb_->capacity_flag = cb_->capacity - 1;
    return 0;
}

void cb_iter_init(cb_iter_t *iter_, cb_t *cb_) {
    memset(iter_, 0, sizeof (cb_iter_t));
    iter_->cb = cb_;
}

void* cb_iter_next(cb_iter_t *iter_) {
    const cb_t *cb = iter_->cb;
    if (cb->count < iter_->index) return NULL;

    void* p;
    /* saturation case */
    if (cb->count == cb->capacity) {
        unsigned off_idx = (cb->index + iter_->index) & cb->capacity_flag;
        p = cb->buffer + (off_idx << cb->pow2_sz);
    }
    else
    {
        p = cb->buffer + (iter_->index << cb->pow2_sz);
    }
    ++(iter_->index);

    return p;
}

void cb_free(cb_t *cb) {
    free(cb->buffer);
    memset(cb, 0, sizeof (cb_t));
}

void cb_push(cb_t *cb_, const void *item_, unsigned len_) {
    memcpy((cb_->buffer + (cb_->index << cb_->pow2_sz)), item_, len_);
    ++(cb_->index);
    if (cb_->count < cb_->capacity_flag) ++(cb_->count);
    cb_->index &= cb_->capacity_flag;
}

