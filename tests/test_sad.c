#include <stdlib.h>
#include <string.h>



#include "sad.h"

static int has_failed = 0;

/* 1,2,3,5,6,8,9,18,19,24,27 */
static char test_sentences[] = {
    /* type 1 */
    "!AIVDM,1,1,,A,139Hgs0000PDSwJMdVUqlop:P>`<,0*1B" "\n"
    /* type 2 */
    "!AIVDM,1,1,,A,23HQ9iP01MPF3evMweQJvpkdP>`<,0*3B" "\r\n"
    /* type 3 */
    "!AIVDM,1,1,,A,36SpOn1000acew4Cl0Go90H<R>`<,0*7E" "\r\n"
    /* type 5 */
    "!AIVDM,1,1,,A,53aGD7l000010COC;<0MDhJ0lE8U@0000000001S30841t@PJ520DS2CQiCP00000000000,2*75" "\r\n"
    /* type 9 */
    "!AIVDM,1,1,,A,90004b@GAlOTK8HNe=;UV>020L0j,0*47" "\n"
    /* type 18 */
    "!AIVDM,1,1,,B,B3aDA8h00839207D=pFFkwrUoP06,0*6C" "\n"
    /* type 19 */
    "!AIVDM,1,1,,A,C6:Veoh40:9q@=4UbO>;;wv0jbj2L?S1111111111110S5`84TRP,0*09" "\n"
    /* type 24 -- TODO */
    /* type 27 -- TODO */
//    "!AIVDM,1,1,,A,K3cov<90Dq3@80:h,0*0D\n"
};

static char* tests_result[] = {
  "{\"class\":\"AIS\",\"type\":1,\"repeat\":0,\"mmsi\":211169260,\"scaled\":false,\"status\":0,\"turn\":0,\"speed\":0,\"accuracy\":true,\"lon\":2695149,\"lat\":31139479,\"course\":2515,\"heading\":252,\"second\":5,\"maneuver\":1,\"raim\":false,\"radio\":119832}",
  "{\"class\":\"AIS\",\"type\":2,\"repeat\":0,\"mmsi\":227035590,\"scaled\":false,\"status\":0,\"turn\":0,\"speed\":93,\"accuracy\":true,\"lon\":2891199,\"lat\":31452549,\"course\":2811,\"heading\":281,\"second\":54,\"maneuver\":1,\"raim\":false,\"radio\":119832}",
  "{\"class\":\"AIS\",\"type\":3,\"repeat\":0,\"mmsi\":440279000,\"scaled\":false,\"status\":1,\"turn\":0,\"speed\":0,\"accuracy\":true,\"lon\":81227746,\"lat\":20775007,\"course\":1828,\"heading\":12,\"second\":6,\"maneuver\":1,\"raim\":true,\"radio\":119832}",
  "{\"class\":\"AIS\",\"type\":5,\"repeat\":0,\"mmsi\":244700191,\"scaled\":false,\"imo\":0,\"ais_version\":1,\"callsign\":\"PD7423\",\"shipname\":\"GULF MERIT\",\"shiptype\":99,\"to_bow\":24,\"to_stern\":8,\"to_port\":4,\"to_starboard\":1,\"epfd\":15,\"eta\":\"01-01T00:26Z\",\"draught\":20,\"destination\":\"HARLINGEN\",\"dte\":0}",
  "{\"class\":\"AIS\",\"type\":9,\"repeat\":0,\"mmsi\":1193,\"scaled\":false,\"alt\":93,\"speed\":116,\"accuracy\":false,\"lon\":-3614452,\"lat\":32197934,\"course\":1432,\"second\":56,\"regional\":0,\"dte\":1,\"raim\":false,\"radio\":57369}",
  "{\"class\":\"AIS\",\"type\":18,\"repeat\":0,\"mmsi\":244650275,\"scaled\":false,\"reserved\":0,\"speed\":0,\"accuracy\":true,\"lon\":1646848,\"lat\":30685061,\"course\":2412,\"heading\":511,\"second\":53,\"regional\":0,\"cs\":true,\"display\":false,\"dsc\":true,\"band\":true,\"msg22\":true,\"raim\":true,\"radio\":917510}",
  "{\"class\":\"AIS\",\"type\":19,\"repeat\":0,\"mmsi\":413773279,\"scaled\":false,\"reserved\":1,\"speed\":0,\"accuracy\":true,\"lon\":72296474,\"lat\":19245555,\"course\":2226,\"heading\":511,\"second\":60,\"regional\":0,\"shipname\":\"YUYANG1\",\"shiptype\":70,\"to_bow\":90,\"to_stern\":16,\"to_port\":9,\"to_starboard\":9,\"epfd\":4,\"raim\":false,\"dte\":0,\"assigned\":true}",
};


static int on_ais_decoded(struct sad_filter_s * filter_){
    static int count = 0;
    char buf[4096] = {0};
    memcpy(buf,filter_->mess->start,filter_->mess->n );
    
    char bufjson[4096] = {0};
    json_aivdm_dump(&filter_->ais, NULL, 0, bufjson, sizeof(bufjson));
    
    if(0 == strncmp(tests_result[count], bufjson, strlen(tests_result[count]))){        
        printf("[PASS] %s\n", buf);
        printf("        %s", bufjson);
    }else
    {
        printf("[FAIL] %s (%zu:%zu)\n", buf, strlen(bufjson), strlen(tests_result[count]));
        has_failed = 1;
        printf("    ---%s    +++%s\n", bufjson, tests_result[count]);
    }
    ++count;
    
    
    return 0;
}

int main(int argc, char **argv) {
    (void) argv;
    (void) argc;
    sad_filter_t filter;
    if (sad_filter_init(&filter, on_ais_decoded, NULL, NULL)) return 1;
    sad_decode_multiline(&filter, test_sentences, sizeof(test_sentences));

    return has_failed;
}

