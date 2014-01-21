#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmtest.h"
#include "mmcb.h"

MMTEST_DECL

int main(int argc, char** argv) {
    (void) argc;

    /* test 0 */
    {
        cb_t cb;
        MMTEST((0 == cb_init(&cb, 2, 7)));

        char buffer[2 << 7];

        int i;
        for (i = 0; i < cb.capacity; ++i) {
            sprintf(buffer, "string_%d", i);
            cb_push(&cb, (void*) buffer, strlen(buffer));
            MMTEST(i == (cb.count + 1));
        }

        /* none full circular buffer iter */
        {
            cb_iter_t iter;
            cb_iter_init(&iter, &cb);
            void * p;

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_0", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_1", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_2", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_3", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((NULL == p))
        }

        /* full circular buffer iter */
        {
            cb_iter_t iter;
            cb_iter_init(&iter, &cb);
            void * p;

            for (i = 4; i < 6; ++i) {
                sprintf(buffer, "string_%d", i);
                cb_push(&cb, (void*) buffer, strlen(buffer));
            }

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_4", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_5", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_2", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_3", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((NULL == p))
        }
        
        /* full circular buffer iter */
        {
            cb_iter_t iter;
            cb_iter_init(&iter, &cb);
            void * p;

            for (i = 6; i < 10; ++i) {
                sprintf(buffer, "string_%d", i);
                cb_push(&cb, (void*) buffer, strlen(buffer));
            }

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_8", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_9", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_6", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((0 == strcmp("string_7", (char*) p)))

            p = cb_iter_next(&iter);
            MMTEST((NULL == p))
        }        

        



    }

    MMTEST_END
}

