#include <stdio.h>
#include <string.h>

#include "mmtest.h"
#include "mmpool.h"

struct tst_pool {
    int i;
    char name[2048];
};

static int name_cmp_cb(void* l_, void* r_) {
    return (strcmp(((struct tst_pool*) l_)->name, (const char*) r_));
}

MMTEST_DECL



int main(int argc, char **argv) {
    (void) argc;

    /* mmpool_new */
    {
        mmpool_t* pool = mmpool_new(2, 10, 4, sizeof (struct tst_pool), NULL);
        MMTEST((NULL != pool));
        MMTEST((2 == pool->alloc_count));
        MMTEST((10 == pool->capacity));
        MMTEST((4 == pool->alloc_step));
        MMTEST((0 == pool->taken_count));

        /* take 0x */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL != item));
            MMTEST((2 == pool->alloc_count));
            MMTEST((1 == pool->taken_count));
            struct tst_pool* tp = (struct tst_pool*) item->p;
            strcpy(tp->name, "take0x");
        }

        /* take 1x */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL != item));
            MMTEST((2 == pool->alloc_count));
            MMTEST((2 == pool->taken_count));
            struct tst_pool* tp = (struct tst_pool*) item->p;
            strcpy(tp->name, "take1x");
        }

        /* take 2x */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL != item));
            MMTEST((6 == pool->alloc_count));
            MMTEST((3 == pool->taken_count));
            struct tst_pool* tp = (struct tst_pool*) item->p;
            strcpy(tp->name, "take2x");
        }

        /* take 3x */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL != item));
            MMTEST((6 == pool->alloc_count));
            MMTEST((4 == pool->taken_count));
            struct tst_pool* tp = (struct tst_pool*) item->p;
            strcpy(tp->name, "take3x");
        }

        /* take 4x */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL != item));
            MMTEST((6 == pool->alloc_count));
            MMTEST((5 == pool->taken_count));
            struct tst_pool* tp = (struct tst_pool*) item->p;
            strcpy(tp->name, "take4x");
        }

        /* take 5x */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL != item));
            MMTEST((6 == pool->alloc_count));
            MMTEST((6 == pool->taken_count));
            struct tst_pool* tp = (struct tst_pool*) item->p;
            strcpy(tp->name, "take5x");
        }

        /* take 6x */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL != item));
            MMTEST((10 == pool->alloc_count));
            MMTEST((7 == pool->taken_count));
            struct tst_pool* tp = (struct tst_pool*) item->p;
            strcpy(tp->name, "take6x");
        }

        /* take 7x */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL != item));
            MMTEST((10 == pool->alloc_count));
            MMTEST((8 == pool->taken_count));
            struct tst_pool* tp = (struct tst_pool*) item->p;
            strcpy(tp->name, "take7x");
        }

        /* take 8x */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL != item));
            MMTEST((10 == pool->alloc_count));
            MMTEST((9 == pool->taken_count));
            struct tst_pool* tp = (struct tst_pool*) item->p;
            strcpy(tp->name, "take8x");
        }

        /* take 9x */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL != item));
            MMTEST((10 == pool->alloc_count));
            MMTEST((10 == pool->taken_count));
            struct tst_pool* tp = (struct tst_pool*) item->p;
            strcpy(tp->name, "take9x");
        }

        /* take 10x */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL == item));
        }

        /* test iter */
        {
            static char* expected[] = {
                "take0x", "take1x", "take2x", "take3x", "take4x", "take5x",
                "take6x", "take7x", "take8x", "take9x"
            };
            mmpool_iter_t iter;
            mmpool_iter_init(iter, pool);
            struct tst_pool* tp = (struct tst_pool*) mmpool_iter_next(&iter);
            unsigned int i = 0;
            while (tp) {
                MMTEST((0 == strcmp(expected[i], tp->name)));
                tp = (struct tst_pool*) mmpool_iter_next(&iter);
                ++i;
            }
        }

        /* test finder */
        {
            mmpool_finder_t finder;
            mmpool_finder_init(finder, pool, name_cmp_cb);

            mmpool_item_t* found_item = mmpool_find(&finder, (void*) "take3x");
            MMTEST((NULL != found_item));
            struct tst_pool* tp = (struct tst_pool*) found_item->p;
            MMTEST((NULL != tp));
            MMTEST(0 == strcmp("take3x", tp->name));

            /* test giveback*/
            {
                mmpool_giveback(found_item);
                MMTEST((10 == pool->alloc_count));
                MMTEST((9 == pool->taken_count));

                /* test expected after giveback */
                {
                    static char* expected2[] = {
                        "take0x", "take1x", "take2x", "take4x", "take5x",
                        "take6x", "take7x", "take8x", "take9x"
                    };
                    mmpool_iter_t iter;
                    mmpool_iter_init(iter, pool);
                    struct tst_pool* tp = (struct tst_pool*) mmpool_iter_next(&iter);
                    unsigned int i = 0;
                    while (tp) {
                        MMTEST((0 == strcmp(expected2[i], tp->name)));
                        tp = (struct tst_pool*) mmpool_iter_next(&iter);
                        ++i;
                    }
                }
            }

            /* take ax */
            {
                mmpool_item_t* itema = mmpool_take(pool);
                MMTEST((NULL != itema));
                MMTEST((10 == pool->alloc_count));
                MMTEST((10 == pool->taken_count));
                struct tst_pool* tp = (struct tst_pool*) itema->p;
                strcpy(tp->name, "takeax");

                /* test expected after retaken */
                {
                    static char* expected3[] = {
                        "take0x", "take1x", "take2x", "takeax", "take4x", "take5x",
                        "take6x", "take7x", "take8x", "take9x"
                    };
                    mmpool_iter_t iter;
                    mmpool_iter_init(iter, pool);
                    struct tst_pool* tp = (struct tst_pool*) mmpool_iter_next(&iter);
                    unsigned int i = 0;
                    while (tp) {
                        MMTEST((0 == strcmp(expected3[i], tp->name)));
                        tp = (struct tst_pool*) mmpool_iter_next(&iter);
                        ++i;
                    }
                }
            }
        }
        

        /* take bx */
        {
            mmpool_item_t* item = mmpool_take(pool);
            MMTEST((NULL == item));
        }    

        mmpool_free(pool);
    }

    /* mmpool_easy_new */
    {
        mmpool_t* pool = mmpool_easy_new(10, sizeof (struct tst_pool), NULL);
        MMTEST((NULL != pool));
        MMTEST((10 == pool->alloc_count));
        MMTEST((10 == pool->capacity));
        MMTEST((1 == pool->alloc_step));
        mmpool_free(pool);
    }

    MMTEST_END
}

