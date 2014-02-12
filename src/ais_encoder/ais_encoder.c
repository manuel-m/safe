#include <stdlib.h>
#include <string.h>
#include "sad.h"
#include "tinybits.h"


#define AIVDM_PREFIX "!AIVDM,1,1,,A,"

/*
 * |============================================================================
 * |Field   |Len |Description             |Member    |T|Units
 * |0-5     | 6  |Message Type            |type      |u|Constant: 1-3
 * |6-7     | 2  |Repeat Indicator        |repeat    |u|Message repeat count
 * |8-37    |30  |MMSI                    |mmsi      |u|9 decimal digits
 * |38-41   | 4  |Navigation Status       |status    |e|See "Navigation Status"
 * |42-49   | 8  |Rate of Turn (ROT)      |turn      |I3|See below
 * |50-59   |10  |Speed Over Ground (SOG) |speed     |U1|See below
 * |60-60   | 1  |Position Accuracy       |accuracy  |b|See below
 * |61-88   |28  |Longitude               |lon       |I4|Minutes/10000 (see below)
 * |89-115  |27  |Latitude                |lat       |I4|Minutes/10000 (see below)
 * |116-127 |12  |Course Over Ground (COG)|course    |U1|Relative to true north,
 *                                                      to 0.1 degree precision
 * |128-136 | 9  |True Heading (HDG)      |heading   |u|0 to 359 degrees,
 *                                                       511 = not available.
 * |137-142 | 6  |Time Stamp              |second    |u|Second of UTC timestamp
 * |143-144 | 2  |Maneuver Indicator      |maneuver  |e|See "Maneuver Indicator"
 * |145-147 | 3  |Spare                   |          |x|Not used
 * |148-148 | 1  |RAIM flag               |raim      |b|See below
 * |149-167 |19  |Radio status            |radio     |u|See below
 * |============================================================================
 */

struct __attribute__((packed)) pack_mess1_s {
    int type : 6;
    int repeat : 2;
    int mmsi : 30;
    int status : 4;
    int turn : 8;
    int speed : 10;
    int accuracy : 1;
    int lon : 28;
    int lat : 27;
    int course : 12;
    int heading : 9;
    int second : 6;
    int maneuver : 2;
    int spare : 3;
    int raim : 1;
    int radio : 19;
};

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    struct pack_mess1_s pack_mess1 = {
        .type = 1,
        .repeat = 0,
        .mmsi = 211169260,
        .status = 0,
        .turn = 0,
        .accuracy = 1,
        .lon = 2695149,
        .lat = 31139479,
        .course = 2515,
        .heading = 252,
        .second = 5,
        .maneuver = 1,
        .spare = 0,
        .raim = 0,
        .radio = 119832
    };

    unsigned char payload[sizeof (pack_mess1)];
    memset(payload, 0, sizeof (payload));

    memcpy(payload, &pack_mess1, sizeof (pack_mess1));

    tb_dump_binary_little(sizeof (payload), payload, ' ');
    puts("");

    tb_dump_binary_big(sizeof (payload), payload, ' ');
    puts("");

    int i;

    unsigned char *p = payload;

    unsigned char p6[1024];
    memset(p6, 0, sizeof (p6));
    //    unsigned char *p6val = p6;

    /* 8 x 3 = 24 = 6 x 4 */
    const char t6[] = "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVW`abcdefghijklmnopqrstuvw";
    (void)t6;

    for (i = 0; i<sizeof (payload); i++) {

        unsigned mod = i % 4;

        if (!mod) {
            printf("%u\tb0\t(%u)\n", *p, *p & 63);
        } else {
            if (1 == mod) {
                unsigned v = ((*p & 192) >> 6) | ((*(p + 1) & 15) << 2);
                printf("%u\tb1\t(%u)\n", *p, v);
            } else {
                if (2 == mod) {
                    printf("%u\tb2\t(%u)\n", *p, 0);

                } else {
                    // mod:3
                    printf("%u\tb3\t(%u)\n", *p, *p & 252 >> 2);
                }
            }
        }
        p++;
    }
    puts("");

    //    unsigned char *p6valiter = p6;
    //    do{
    //        printf("<%c>:(%d)", t6[*p6valiter], *p6valiter);
    //        ++p6valiter;
    //    }while(p6valiter<p6val);
    //    puts("");







    return 0;

}
