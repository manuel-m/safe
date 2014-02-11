#include <stdlib.h>
#include <string.h>
#include "sad.h"



#define AIVDM_PREFIX "!AIVDM,1,1,,A,"
#define AIVDM_PREFIX_LEN 14
#define AIVDM1_PAYLOAD_SIZE 168

/*
|==============================================================================
|Field   |Len |Description             |Member    |T|Units
|0-5     | 6  |Message Type            |type      |u|Constant: 1-3
|6-7     | 2  |Repeat Indicator        |repeat    |u|Message repeat count
|8-37    |30  |MMSI                    |mmsi      |u|9 decimal digits
|38-41   | 4  |Navigation Status       |status    |e|See "Navigation Status"
|42-49   | 8  |Rate of Turn (ROT)      |turn      |I3|See below
|50-59   |10  |Speed Over Ground (SOG) |speed     |U1|See below
|60-60   | 1  |Position Accuracy       |accuracy  |b|See below
|61-88   |28  |Longitude               |lon       |I4|Minutes/10000 (see below)
|89-115  |27  |Latitude                |lat       |I4|Minutes/10000 (see below)
|116-127 |12  |Course Over Ground (COG)|course    |U1|Relative to true north,
                                                     to 0.1 degree precision
|128-136 | 9  |True Heading (HDG)      |heading   |u|0 to 359 degrees,
                                                      511 = not available.
|137-142 | 6  |Time Stamp              |second    |u|Second of UTC timestamp
|143-144 | 2  |Maneuver Indicator      |maneuver  |e|See "Maneuver Indicator"
|145-147 | 3  |Spare                   |          |x|Not used
|148-148 | 1  |RAIM flag               |raim      |b|See below
|149-167 |19  |Radio status            |radio     |u|See below
|==============================================================================   
 */


void unsigned2_bits(unsigned char* vect_, unsigned in_, unsigned offset_){
    
    
    
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    unsigned mmsi = 211169260;
    
const char str6b[64] =
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^- !\"#$%&'()*+,-./0123456789:;<=>?"; 

    unsigned count = 5;
    char sentence[5 + 1] = {0};

    char * p = sentence + count;

    *p = '\0';
    --p;

#define xxxMMM6(P,VAR)                                                         \
do{                                                                            \
  unsigned k = (VAR) & 63u;printf("%u\n",k);                                   \
  *(P) = str6b[k];                                                             \
  (VAR) >>= 6;                                                                 \
  --(P);                                                                       \
}while(0);    

    xxxMMM6(p, mmsi);
    xxxMMM6(p, mmsi);
    xxxMMM6(p, mmsi);
    xxxMMM6(p, mmsi);
    xxxMMM6(p, mmsi);
    
    printf("%s\n", sentence);





    /*    
    // !AIVDM,1,1,,A,139Hgs0000PDSwJMdVUqlop:P>`<,0*1B"   
       //    memset(&ais, 0, sizeof (ais));
       struct ais_t ais = {
           .mmsi = 211169260,
           .repeat = 0,
           .type = 1,
           .type1.accuracy = 1,
           .type1.course = 2515,
           .type1.heading = 252,
           .type1.lat = 31139479,
           .type1.lon = 2695149,
           .type1.maneuver = 1,
           .type1.radio = 119832,
           .type1.raim = 0,
           .type1.second = 5,
           .type1.speed = 0,
           .type1.status = 0,
           .type1.turn = 0
       };    


    
       unsigned char bits[AIVDM1_PAYLOAD_SIZE];
       unsigned char * pbits = bits;
       memset(bits,0, sizeof(bits));
   
       #define MMPBIT(xxVALUExx, xxLENxx)                                          \
       do{                                                                         \
           memcpy(pbits,&(xxVALUExx),xxLENxx );                                    \
           pbits += xxLENxx;                                                       \
       } while(0);
   
       MMPBIT(ais.type,6);                            // message type
       MMPBIT(ais.repeat,2);                          // repeat indicator
       MMPBIT(ais.mmsi, 30);                          // mmsi
       MMPBIT(ais.type1.status,4);                    // navigation status
       MMPBIT(ais.type1.turn,8);                      // rate of turn
       MMPBIT(ais.type1.speed,10);                    // speed over ground
       MMPBIT(ais.type1.accuracy,1);                  // position accuracy
       MMPBIT(ais.type1.lon,28);                      // longitude
       MMPBIT(ais.type1.lat,27);                      // latitude
       MMPBIT(ais.type1.course,12);                   // course over ground
       MMPBIT(ais.type1.heading,9);                   // true heading
       MMPBIT(ais.type1.second,6);                    // timestamp
       MMPBIT(ais.type1.maneuver,2);                  // maneuver indicator
       pbits += 3;                                    // spare
       MMPBIT(ais.type1.raim,1);                      // raim flag
     *pbits = (unsigned char)(ais.type1.radio);     // radio
   
       pbits = bits;
   
       char payload[127];
       memset(payload,0, sizeof(payload));
       char * ppayload = payload;
   
       while(pbits < (bits + AIVDM1_PAYLOAD_SIZE)){
           unsigned char x = ((*pbits) & 64U) + 48U;
           printf("(%u:%p)%u\n",*pbits, pbits, x);
     *ppayload = (char)x;
           pbits += 6; 
           ++ppayload;
       }

        printf("%s%s\n", AIVDM_PREFIX,payload);
    
    
     */
    return 0;

}
