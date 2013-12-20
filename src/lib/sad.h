
#ifndef _AISD_H_
#define _AISD_H_

#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#include "sub0.h"

#ifdef __cplusplus
extern "C" {
#endif


// #define SAD_ENABLE_ALL 1

#ifdef SAD_ENABLE_ALL
#  define SAD_ENABLE_MESSAGE_1 1
#  define SAD_ENABLE_MESSAGE_2 1
#  define SAD_ENABLE_MESSAGE_3 1
#  define SAD_ENABLE_MESSAGE_4 1
#  define SAD_ENABLE_MESSAGE_5 1
#  define SAD_ENABLE_MESSAGE_6 1
#  define SAD_ENABLE_MESSAGE_7 1
#  define SAD_ENABLE_MESSAGE_8 1
#  define SAD_ENABLE_MESSAGE_9 1
#  define SAD_ENABLE_MESSAGE_10 1
#  define SAD_ENABLE_MESSAGE_11 1
#  define SAD_ENABLE_MESSAGE_12 1
#  define SAD_ENABLE_MESSAGE_13 1
#  define SAD_ENABLE_MESSAGE_14 1
#  define SAD_ENABLE_MESSAGE_15 1
#  define SAD_ENABLE_MESSAGE_16 1
#  define SAD_ENABLE_MESSAGE_17 1
#  define SAD_ENABLE_MESSAGE_18 1
#  define SAD_ENABLE_MESSAGE_19 1
#  define SAD_ENABLE_MESSAGE_20 1
#  define SAD_ENABLE_MESSAGE_21 1
#  define SAD_ENABLE_MESSAGE_22 1
#  define SAD_ENABLE_MESSAGE_23 1
#  define SAD_ENABLE_MESSAGE_24 1
#  define SAD_ENABLE_MESSAGE_25 1
#  define SAD_ENABLE_MESSAGE_26 1
#  define SAD_ENABLE_MESSAGE_27 1
#endif



#define AIVDM_ENABLE 1
#define CHEAPFLOATS_ENABLE 1
#define NMEA_ENABLE 1
#define AIVDM_MESSAGES_TYPE 27    

    /* will not handle pre-Intel Apples that can run big-endian 
       __BIG_ENDIAN__ and __LITTLE_ENDIAN__ are define in some gcc versions
      only, probably depending on the architecture. Try to use endian.h if
      the gcc way fails - endian.h also doesn not seem to be available on all
      platforms.
     */
#ifndef _WIN32    
#ifdef __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#else /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
#undef WORDS_BIGENDIAN
#else
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#undef WORDS_BIGENDIAN
#else
#error "unable to determine endianess!"
#endif /* __BYTE_ORDER */
#endif /* __LITTLE_ENDIAN__ */
#endif /* __BIG_ENDIAN__ */
#endif /* not WIN32 */
    /* Some libcs do not have strlcat/strlcpy. Local copies are provided */
#ifndef HAVE_STRLCAT
    size_t strlcat(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
#endif

#ifndef HAVE_STRLCPY

    size_t strlcpy(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);


#endif


#ifdef _WIN32
    typedef unsigned int speed_t;
#endif

#define JSON_DATE_MAX	24	/* ISO8601 timestamp with 2 decimal places */


    /* First, declarations for the packet layer... */

    /*
     * For NMEA-conforming receivers this is supposed to be 82, but
     * some receivers (TN-200, GSW 2.3.2) emit oversized sentences.
     * The current hog champion is the Trimble BX-960 receiver, which
     * emits a 91-character GGA message.
     */
#define NMEA_MAX	91		/* max length of NMEA sentence */
#define NMEA_BIG_BUF	(2*NMEA_MAX+1)	/* longer than longest NMEA sentence */


    /*
     * The packet buffers need to be as long than the longest packet we
     * expect to see in any protocol, because we have to be able to hold
     * an entire packet for checksumming...
     * First we thought it had to be big enough for a SiRF Measured Tracker
     * Data packet (188 bytes). Then it had to be big enough for a UBX SVINFO
     * packet (206 bytes). Now it turns out that a couple of ITALK messages are
     * over 512 bytes. I know we like verbose output, but this is ridiculous.
     */
#define MAX_PACKET_LENGTH	516	/* 7 + 506 + 3 */

    struct gps_packet_t {
        /* packet-getter internals */
        int type;
#define BAD_PACKET      	-1
#define COMMENT_PACKET  	0
#define NMEA_PACKET     	1
#define AIVDM_PACKET    	2
#define GARMINTXT_PACKET	3
#define MAX_TEXTUAL_TYPE	3	/* increment this as necessary */
#define SIRF_PACKET     	4
#define ZODIAC_PACKET   	5
#define TSIP_PACKET     	6
#define EVERMORE_PACKET 	7
#define ITALK_PACKET    	8
#define GARMIN_PACKET   	9
#define NAVCOM_PACKET   	10
#define UBX_PACKET      	11
#define SUPERSTAR2_PACKET	12
#define ONCORE_PACKET   	13
#define GEOSTAR_PACKET   	14
#define NMEA2000_PACKET 	15
#define MAX_GPSPACKET_TYPE	15	/* increment this as necessary */
#define RTCM2_PACKET    	16
#define RTCM3_PACKET    	17
#define JSON_PACKET    	    	18
#define TEXTUAL_PACKET_TYPE(n)	((((n)>=NMEA_PACKET) && ((n)<=MAX_TEXTUAL_TYPE)) || (n)==JSON_PACKET)
#define GPS_PACKET_TYPE(n)	(((n)>=NMEA_PACKET) && ((n)<=MAX_GPSPACKET_TYPE))
#define LOSSLESS_PACKET_TYPE(n)	(((n)>=RTCM2_PACKET) && ((n)<=RTCM3_PACKET))
#define PACKET_TYPEMASK(n)	(1 << (n))
#define GPS_TYPEMASK	(((2<<(MAX_GPSPACKET_TYPE+1))-1) &~ PACKET_TYPEMASK(COMMENT_PACKET))
        unsigned int state;
        size_t inbuflen;
        unsigned /*@observer@*/char *inbufptr;
        /* outbuffer needs to be able to hold 4 GPGSV records at once */
        unsigned char outbuffer[MAX_PACKET_LENGTH * 2 + 1];
        size_t outbuflen;
        size_t length;
        unsigned char inbuffer[MAX_PACKET_LENGTH * 2 + 1];
        unsigned long char_counter; /* count characters processed */
        unsigned long retry_counter; /* count sniff retries */
        unsigned counter; /* packets since last driver switch */
        int debug; /* lexer debug level */


    };

#define packet_buffered_input(lexer) ((lexer)->inbuffer + (lexer)->inbuflen - (lexer)->inbufptr)


    /* Next, declarations for the core library... */
    /*
     * Is an MMSI number that of an auxiliary associated with a mother ship?
     * We need to be able to test this for decoding AIS Type 24 messages.
     * According to <http://www.navcen.uscg.gov/marcomms/gmdss/mmsi.htm#format>,
     * auxiliary-craft MMSIs have the form 98MIDXXXX, where MID is a country
     * code and XXXX the vessel ID.
     */
#define AIS_AUXILIARY_MMSI(n)	((n) / 10000000 == 98)

    /* N/A values and scaling constant for 25/24 bit lon/lat pairs */
#define AIS_LON3_NOT_AVAILABLE	181000
#define AIS_LAT3_NOT_AVAILABLE	91000
#define AIS_LATLON3_DIV	60000.0

    /* N/A values and scaling constant for 28/27 bit lon/lat pairs */
#define AIS_LON4_NOT_AVAILABLE	1810000
#define AIS_LAT4_NOT_AVAILABLE	910000
#define AIS_LATLON4_DIV	600000.0

    struct route_info {
        unsigned int linkage; /* Message Linkage ID */
        unsigned int sender; /* Sender Class */
        unsigned int rtype; /* Route Type */
        unsigned int month; /* Start month */
        unsigned int day; /* Start day */
        unsigned int hour; /* Start hour */
        unsigned int minute; /* Start minute */
        unsigned int duration; /* Duration */
        int waycount; /* Waypoint count */

        struct waypoint_t {
            signed int lon; /* Longitude */
            signed int lat; /* Latitude */
        } waypoints[16];
    };


#define AIVDM_CHANNELS	2		/* A, B */
    
           
        typedef struct type123_s {
                unsigned int status; /* navigation status */
                signed turn; /* rate of turn */
#define AIS_TURN_HARD_LEFT	-127
#define AIS_TURN_HARD_RIGHT	127
#define AIS_TURN_NOT_AVAILABLE	128
                unsigned int speed; /* speed over ground in deciknots */
#define AIS_SPEED_NOT_AVAILABLE	1023
#define AIS_SPEED_FAST_MOVER	1022		/* >= 102.2 knots */
                bool accuracy; /* position accuracy */
#define AIS_LATLON_DIV	600000.0
                int lon; /* longitude */
#define AIS_LON_NOT_AVAILABLE	0x6791AC0
                int lat; /* latitude */
#define AIS_LAT_NOT_AVAILABLE	0x3412140
                unsigned int course; /* course over ground */
#define AIS_COURSE_NOT_AVAILABLE	3600
                unsigned int heading; /* true heading */
#define AIS_HEADING_NOT_AVAILABLE	511
                unsigned int second; /* seconds of UTC timestamp */
#define AIS_SEC_NOT_AVAILABLE	60
#define AIS_SEC_MANUAL		61
#define AIS_SEC_ESTIMATED	62
#define AIS_SEC_INOPERATIVE	63
                unsigned int maneuver; /* maneuver indicator */
                bool raim; /* RAIM flag */
                unsigned int radio; /* radio status bits */
            } type123_t;    

    struct ais_t {
        unsigned int type; /* message type */
        unsigned int repeat; /* Repeat indicator */
        unsigned int mmsi; /* MMSI */

        union {

            /* Types 1-3 Common navigation info */
            type123_t type1;

            /* Type 4 - Base Station Report & Type 11 - UTC and Date Response */
            struct {
                unsigned int year; /* UTC year */
#define AIS_YEAR_NOT_AVAILABLE	0
                unsigned int month; /* UTC month */
#define AIS_MONTH_NOT_AVAILABLE	0
                unsigned int day; /* UTC day */
#define AIS_DAY_NOT_AVAILABLE	0
                unsigned int hour; /* UTC hour */
#define AIS_HOUR_NOT_AVAILABLE	24
                unsigned int minute; /* UTC minute */
#define AIS_MINUTE_NOT_AVAILABLE	60
                unsigned int second; /* UTC second */
#define AIS_SECOND_NOT_AVAILABLE	60
                bool accuracy; /* fix quality */
                int lon; /* longitude */
                int lat; /* latitude */
                unsigned int epfd; /* type of position fix device */
                bool raim; /* RAIM flag */
                unsigned int radio; /* radio status bits */
            } type4;

            /* Type 5 - Ship static and voyage related data */
            struct {
                unsigned int ais_version; /* AIS version level */
                unsigned int imo; /* IMO identification */
                char callsign[7 + 1]; /* callsign */
#define AIS_SHIPNAME_MAXLEN	20
                char shipname[AIS_SHIPNAME_MAXLEN + 1]; /* vessel name */
                unsigned int shiptype; /* ship type code */
                unsigned int to_bow; /* dimension to bow */
                unsigned int to_stern; /* dimension to stern */
                unsigned int to_port; /* dimension to port */
                unsigned int to_starboard; /* dimension to starboard */
                unsigned int epfd; /* type of position fix deviuce */
                unsigned int month; /* UTC month */
                unsigned int day; /* UTC day */
                unsigned int hour; /* UTC hour */
                unsigned int minute; /* UTC minute */
                unsigned int draught; /* draft in meters */
                char destination[20 + 1]; /* ship destination */
                unsigned int dte; /* data terminal enable */
            } type5;

            /* Type 6 - Addressed Binary Message */
            struct {
                unsigned int seqno; /* sequence number */
                unsigned int dest_mmsi; /* destination MMSI */
                bool retransmit; /* retransmit flag */
                unsigned int dac; /* Application ID */
                unsigned int fid; /* Functional ID */
#define AIS_TYPE6_BINARY_MAX	920	/* 920 bits */
                size_t bitcount; /* bit count of the data */

                union {
                    char bitdata[(AIS_TYPE6_BINARY_MAX + 7) / 8];

                    /* GLA - AtoN monitoring data (UK/ROI) */
                    struct {
                        unsigned int ana_int; /* Analogue (internal) */
                        unsigned int ana_ext1; /* Analogue (external #1) */
                        unsigned int ana_ext2; /* Analogue (external #2) */
                        unsigned int racon; /* RACON status */
                        unsigned int light; /* Light status */
                        bool alarm; /* Health alarm*/
                        unsigned int stat_ext; /* Status bits (external) */
                        bool off_pos; /* Off position status */
                    } dac235fid10;

                    /* IMO236 - Dangerous Cargo Indication */
                    struct {
                        char lastport[5 + 1]; /* Last Port Of Call */
                        unsigned int lmonth; /* ETA month */
                        unsigned int lday; /* ETA day */
                        unsigned int lhour; /* ETA hour */
                        unsigned int lminute; /* ETA minute */
                        char nextport[5 + 1]; /* Next Port Of Call */
                        unsigned int nmonth; /* ETA month */
                        unsigned int nday; /* ETA day */
                        unsigned int nhour; /* ETA hour */
                        unsigned int nminute; /* ETA minute */
                        char dangerous[20 + 1]; /* Main Dangerous Good */
                        char imdcat[4 + 1]; /* IMD Category */
                        unsigned int unid; /* UN Number */
                        unsigned int amount; /* Amount of Cargo */
                        unsigned int unit; /* Unit of Quantity */
                    } dac1fid12;

                    /* IMO236 - Extended Ship Static and Voyage Related Data */
                    struct {
                        unsigned int airdraught; /* Air Draught */
                    } dac1fid15;

                    /* IMO236 - Number of Persons on board */
                    struct {
                        unsigned persons; /* number of persons */
                    } dac1fid16;

                    /* IMO289 - Clearance Time To Enter Port */
                    struct {
                        unsigned int linkage; /* Message Linkage ID */
                        unsigned int month; /* Month (UTC) */
                        unsigned int day; /* Day (UTC) */
                        unsigned int hour; /* Hour (UTC) */
                        unsigned int minute; /* Minute (UTC) */
                        char portname[20 + 1]; /* Name of Port & Berth */
                        char destination[5 + 1]; /* Destination */
                        signed int lon; /* Longitude */
                        signed int lat; /* Latitude */
                    } dac1fid18;

                    /* IMO289 - Berthing Data (addressed) */
                    struct {
                        unsigned int linkage; /* Message Linkage ID */
                        unsigned int berth_length; /* Berth length */
                        unsigned int berth_depth; /* Berth Water Depth */
                        unsigned int position; /* Mooring Position */
                        unsigned int month; /* Month (UTC) */
                        unsigned int day; /* Day (UTC) */
                        unsigned int hour; /* Hour (UTC) */
                        unsigned int minute; /* Minute (UTC) */
                        unsigned int availability; /* Services Availability */
                        unsigned int agent; /* Agent */
                        unsigned int fuel; /* Bunker/fuel */
                        unsigned int chandler; /* Chandler */
                        unsigned int stevedore; /* Stevedore */
                        unsigned int electrical; /* Electrical */
                        unsigned int water; /* Potable water */
                        unsigned int customs; /* Customs house */
                        unsigned int cartage; /* Cartage */
                        unsigned int crane; /* Crane(s) */
                        unsigned int lift; /* Lift(s) */
                        unsigned int medical; /* Medical facilities */
                        unsigned int navrepair; /* Navigation repair */
                        unsigned int provisions; /* Provisions */
                        unsigned int shiprepair; /* Ship repair */
                        unsigned int surveyor; /* Surveyor */
                        unsigned int steam; /* Steam */
                        unsigned int tugs; /* Tugs */
                        unsigned int solidwaste; /* Waste disposal (solid) */
                        unsigned int liquidwaste; /* Waste disposal (liquid) */
                        unsigned int hazardouswaste; /* Waste disposal (hazardous) */
                        unsigned int ballast; /* Reserved ballast exchange */
                        unsigned int additional; /* Additional services */
                        unsigned int regional1; /* Regional reserved 1 */
                        unsigned int regional2; /* Regional reserved 2 */
                        unsigned int future1; /* Reserved for future */
                        unsigned int future2; /* Reserved for future */
                        char berth_name[20 + 1]; /* Name of Berth */
                        signed int berth_lon; /* Longitude */
                        signed int berth_lat; /* Latitude */
                    } dac1fid20;
                    /* IMO289 - Weather observation report from ship */

                    /*** WORK IN PROGRESS - NOT YET DECODED ***/
                    struct {
                        bool wmo; /* true if WMO variant */

                        union {

                            struct {
                                char location[20 + 1]; /* Location */
                                signed int lon; /* Longitude */
                                signed int lat; /* Latitude */
                                unsigned int day; /* Report day */
                                unsigned int hour; /* Report hour */
                                unsigned int minute; /* Report minute */
                                bool vislimit; /* Max range? */
                                unsigned int visibility; /* Units of 0.1 nm */
#define DAC8FID21_VISIBILITY_NOT_AVAILABLE	127
#define DAC8FID21_VISIBILITY_SCALE		10.0
                                unsigned humidity; /* units of 1% */
                                unsigned int wspeed; /* average wind speed */
                                unsigned int wgust; /* wind gust */
#define DAC8FID21_WSPEED_NOT_AVAILABLE		127
                                unsigned int wdir; /* wind direction */
#define DAC8FID21_WDIR_NOT_AVAILABLE		360
                                unsigned int pressure; /* air pressure, hpa */
#define DAC8FID21_NONWMO_PRESSURE_NOT_AVAILABLE	403
#define DAC8FID21_NONWMO_PRESSURE_HIGH		402	/* > 1200hPa */
#define DAC8FID21_NONWMO_PRESSURE_OFFSET		400	/* N/A */
                                unsigned int pressuretend; /* tendency */
                                int airtemp; /* temp, units 0.1C */
#define DAC8FID21_AIRTEMP_NOT_AVAILABLE		-1024
#define DAC8FID21_AIRTEMP_SCALE			10.0
                                unsigned int watertemp; /* units 0.1degC */
#define DAC8FID21_WATERTEMP_NOT_AVAILABLE	501
#define DAC8FID21_WATERTEMP_SCALE		10.0
                                unsigned int waveperiod; /* in seconds */
#define DAC8FID21_WAVEPERIOD_NOT_AVAILABLE	63
                                unsigned int wavedir; /* direction in deg */
#define DAC8FID21_WAVEDIR_NOT_AVAILABLE		360
                                unsigned int swellheight; /* in decimeters */
                                unsigned int swellperiod; /* in seconds */
                                unsigned int swelldir; /* direction in deg */
                            } nonwmo_obs;

                            struct {
                                signed int lon; /* Longitude */
                                signed int lat; /* Latitude */
                                unsigned int month; /* UTC month */
                                unsigned int day; /* Report day */
                                unsigned int hour; /* Report hour */
                                unsigned int minute; /* Report minute */
                                unsigned int course; /* course over ground */
                                unsigned int speed; /* speed, m/s */
#define DAC8FID21_SOG_NOT_AVAILABLE		31
#define DAC8FID21_SOG_HIGH_SPEED		30
#define DAC8FID21_SOG_SCALE			2.0
                                unsigned int heading; /* true heading */
#define DAC8FID21_HDG_NOT_AVAILABLE		127
#define DAC8FID21_HDG_SCALE			5.0
                                unsigned int pressure; /* units of hPa * 0.1 */
#define DAC8FID21_WMO_PRESSURE_SCALE		10
#define DAC8FID21_WMO_PRESSURE_OFFSET		90.0
                                unsigned int pdelta; /* units of hPa * 0.1 */
#define DAC8FID21_PDELTA_SCALE			10
#define DAC8FID21_PDELTA_OFFSET			50.0
                                unsigned int ptend; /* enumerated */
                                unsigned int twinddir; /* in 5 degree steps */
#define DAC8FID21_TWINDDIR_NOT_AVAILABLE	127
                                unsigned int twindspeed; /* meters per second */
#define DAC8FID21_TWINDSPEED_SCALE		2
#define DAC8FID21_RWINDSPEED_NOT_AVAILABLE	255
                                unsigned int rwinddir; /* in 5 degree steps */
#define DAC8FID21_RWINDDIR_NOT_AVAILABLE	127
                                unsigned int rwindspeed; /* meters per second */
#define DAC8FID21_RWINDSPEED_SCALE		2
#define DAC8FID21_RWINDSPEED_NOT_AVAILABLE	255
                                unsigned int mgustspeed; /* meters per second */
#define DAC8FID21_MGUSTSPEED_SCALE		2
#define DAC8FID21_MGUSTSPEED_NOT_AVAILABLE	255
                                unsigned int mgustdir; /* in 5 degree steps */
#define DAC8FID21_MGUSTDIR_NOT_AVAILABLE	127
                                unsigned int airtemp; /* degress K */
#define DAC8FID21_AIRTEMP_OFFSET		223
                                unsigned humidity; /* units of 1% */
#define DAC8FID21_HUMIDITY_NOT_VAILABLE		127
                                /* some trailing fields are missing */
                            } wmo_obs;
                        };
                    } dac1fid21;
                    /*** WORK IN PROGRESS ENDS HERE ***/

                    /* IMO289 - Dangerous Cargo Indication */
                    struct {
                        unsigned int unit; /* Unit of Quantity */
                        unsigned int amount; /* Amount of Cargo */
                        int ncargos;

                        struct cargo_t {
                            unsigned int code; /* Cargo code */
                            unsigned int subtype; /* Cargo subtype */
                        } cargos[28];
                    } dac1fid25;
                    /* IMO289 - Route info (addressed) */
                    struct route_info dac1fid28;

                    /* IMO289 - Text message (addressed) */
                    struct {
                        unsigned int linkage;
#define AIS_DAC1FID30_TEXT_MAX	154	/* 920 bits of six-bit, plus NUL */
                        char text[AIS_DAC1FID30_TEXT_MAX];
                    } dac1fid30;

                    /* IMO289 & IMO236 - Tidal Window */
                    struct {
                        unsigned int type; /* Message Type */
                        unsigned int repeat; /* Repeat Indicator */
                        unsigned int mmsi; /* Source MMSI */
                        unsigned int seqno; /* Sequence Number */
                        unsigned int dest_mmsi; /* Destination MMSI */
                        signed int retransmit; /* Retransmit flag */
                        unsigned int dac; /* DAC */
                        unsigned int fid; /* FID */
                        unsigned int month; /* Month */
                        unsigned int day; /* Day */
                        signed int ntidals;

                        struct tidal_t {
                            signed int lon; /* Longitude */
                            signed int lat; /* Latitude */
                            unsigned int from_hour; /* From UTC Hour */
                            unsigned int from_min; /* From UTC Minute */
                            unsigned int to_hour; /* To UTC Hour */
                            unsigned int to_min; /* To UTC Minute */
#define DAC1FID32_CDIR_NOT_AVAILABLE		360
                            unsigned int cdir; /* Current Dir. Predicted */
#define DAC1FID32_CSPEED_NOT_AVAILABLE		127
                            unsigned int cspeed; /* Current Speed Predicted */
                        } tidals[3];
                    } dac1fid32;
                };
            } type6;

            /* Type 7 - Binary Acknowledge */
            struct {
                unsigned int mmsi1;
                unsigned int mmsi2;
                unsigned int mmsi3;
                unsigned int mmsi4;
                /* spares ignored, they're only padding here */
            } type7;

            /* Type 8 - Broadcast Binary Message */
            struct {
                unsigned int dac; /* Designated Area Code */
                unsigned int fid; /* Functional ID */
#define AIS_TYPE8_BINARY_MAX	952	/* 952 bits */
                size_t bitcount; /* bit count of the data */

                union {
                    char bitdata[(AIS_TYPE8_BINARY_MAX + 7) / 8];

                    /* IMO236  - Meteorological-Hydrological data
                     * Trial message, not to be used after January 2013
                     * Replaced by IMO289 (DAC 1, FID 31)
                     */
                    struct {
#define DAC1FID11_LATLON_SCALE			1000
                        int lon; /* longitude in minutes * .001 */
#define DAC1FID11_LON_NOT_AVAILABLE		0xFFFFFF
                        int lat; /* latitude in minutes * .001 */
#define DAC1FID11_LAT_NOT_AVAILABLE		0x7FFFFF
                        unsigned int day; /* UTC day */
                        unsigned int hour; /* UTC hour */
                        unsigned int minute; /* UTC minute */
                        unsigned int wspeed; /* average wind speed */
                        unsigned int wgust; /* wind gust */
#define DAC1FID11_WSPEED_NOT_AVAILABLE		127
                        unsigned int wdir; /* wind direction */
                        unsigned int wgustdir; /* wind gust direction */
#define DAC1FID11_WDIR_NOT_AVAILABLE		511
                        unsigned int airtemp; /* temperature, units 0.1C */
#define DAC1FID11_AIRTEMP_NOT_AVAILABLE		2047
#define DAC1FID11_AIRTEMP_OFFSET		600
#define DAC1FID11_AIRTEMP_DIV			10.0
                        unsigned int humidity; /* relative humidity, % */
#define DAC1FID11_HUMIDITY_NOT_AVAILABLE	127
                        unsigned int dewpoint; /* dew point, units 0.1C */
#define DAC1FID11_DEWPOINT_NOT_AVAILABLE	1023
#define DAC1FID11_DEWPOINT_OFFSET		200
#define DAC1FID11_DEWPOINT_DIV		10.0
                        unsigned int pressure; /* air pressure, hpa */
#define DAC1FID11_PRESSURE_NOT_AVAILABLE	511
#define DAC1FID11_PRESSURE_OFFSET		-800
                        unsigned int pressuretend; /* tendency */
#define DAC1FID11_PRESSURETREND_NOT_AVAILABLE	3
                        unsigned int visibility; /* units 0.1 nautical miles */
#define DAC1FID11_VISIBILITY_NOT_AVAILABLE	255
#define DAC1FID11_VISIBILITY_DIV		10.0
                        int waterlevel; /* decimeters */
#define DAC1FID11_WATERLEVEL_NOT_AVAILABLE	511
#define DAC1FID11_WATERLEVEL_OFFSET		100
#define DAC1FID11_WATERLEVEL_DIV		10.0
                        unsigned int leveltrend; /* water level trend code */
#define DAC1FID11_WATERLEVELTREND_NOT_AVAILABLE	3
                        unsigned int cspeed; /* surface current speed in deciknots */
#define DAC1FID11_CSPEED_NOT_AVAILABLE		255
#define DAC1FID11_CSPEED_DIV			10.0
                        unsigned int cdir; /* surface current dir., degrees */
#define DAC1FID11_CDIR_NOT_AVAILABLE		511
                        unsigned int cspeed2; /* current speed in deciknots */
                        unsigned int cdir2; /* current dir., degrees */
                        unsigned int cdepth2; /* measurement depth, m */
#define DAC1FID11_CDEPTH_NOT_AVAILABLE		31
                        unsigned int cspeed3; /* current speed in deciknots */
                        unsigned int cdir3; /* current dir., degrees */
                        unsigned int cdepth3; /* measurement depth, m */
                        unsigned int waveheight; /* in decimeters */
#define DAC1FID11_WAVEHEIGHT_NOT_AVAILABLE	255
#define DAC1FID11_WAVEHEIGHT_DIV		10.0
                        unsigned int waveperiod; /* in seconds */
#define DAC1FID11_WAVEPERIOD_NOT_AVAILABLE	63
                        unsigned int wavedir; /* direction in degrees */
#define DAC1FID11_WAVEDIR_NOT_AVAILABLE		511
                        unsigned int swellheight; /* in decimeters */
                        unsigned int swellperiod; /* in seconds */
                        unsigned int swelldir; /* direction in degrees */
                        unsigned int seastate; /* Beaufort scale, 0-12 */
#define DAC1FID11_SEASTATE_NOT_AVAILABLE	15
                        unsigned int watertemp; /* units 0.1deg Celsius */
#define DAC1FID11_WATERTEMP_NOT_AVAILABLE	1023
#define DAC1FID11_WATERTEMP_OFFSET		100
#define DAC1FID11_WATERTEMP_DIV		10.0
                        unsigned int preciptype; /* 0-7, enumerated */
#define DAC1FID11_PRECIPTYPE_NOT_AVAILABLE	7
                        unsigned int salinity; /* units of 0.1ppt */
#define DAC1FID11_SALINITY_NOT_AVAILABLE	511
#define DAC1FID11_SALINITY_DIV		10.0
                        unsigned int ice; /* is there sea ice? */
#define DAC1FID11_ICE_NOT_AVAILABLE		3
                    } dac1fid11;

                    /* IMO236 - Fairway Closed */
                    struct {
                        char reason[20 + 1]; /* Reason For Closing */
                        char closefrom[20 + 1]; /* Location Of Closing From */
                        char closeto[20 + 1]; /* Location of Closing To */
                        unsigned int radius; /* Radius extension */
#define AIS_DAC1FID13_RADIUS_NOT_AVAILABLE 10001
                        unsigned int extunit; /* Unit of extension */
#define AIS_DAC1FID13_EXTUNIT_NOT_AVAILABLE 0
                        unsigned int fday; /* From day (UTC) */
                        unsigned int fmonth; /* From month (UTC) */
                        unsigned int fhour; /* From hour (UTC) */
                        unsigned int fminute; /* From minute (UTC) */
                        unsigned int tday; /* To day (UTC) */
                        unsigned int tmonth; /* To month (UTC) */
                        unsigned int thour; /* To hour (UTC) */
                        unsigned int tminute; /* To minute (UTC) */
                    } dac1fid13;

                    /* IMO236 - Extended ship and voyage data */
                    struct {
                        unsigned int airdraught; /* Air Draught */
                    } dac1fid15;

                    /* IMO289 - VTS-generated/Synthetic Targets */
                    struct {
                        signed int ntargets;

                        struct target_t {
#define DAC1FID17_IDTYPE_MMSI		0
#define DAC1FID17_IDTYPE_IMO		1
#define DAC1FID17_IDTYPE_CALLSIGN	2
#define DAC1FID17_IDTYPE_OTHER		3
                            unsigned int idtype; /* Identifier type */

                            union target_id { /* Target identifier */
                                unsigned int mmsi;
                                unsigned int imo;
#define DAC1FID17_ID_LENGTH		7
                                char callsign[DAC1FID17_ID_LENGTH + 1];
                                char other[DAC1FID17_ID_LENGTH + 1];
                            } id;
                            signed int lat; /* Latitude */
                            signed int lon; /* Longitude */
#define DAC1FID17_COURSE_NOT_AVAILABLE		360
                            unsigned int course; /* Course Over Ground */
                            unsigned int second; /* Time Stamp */
#define DAC1FID17_SPEED_NOT_AVAILABLE		255
                            unsigned int speed; /* Speed Over Ground */
                        } targets[4];
                    } dac1fid17;

                    /* IMO 289 - Marine Traffic Signal */
                    struct {
                        unsigned int linkage; /* Message Linkage ID */
                        char station[20 + 1]; /* Name of Signal Station */
                        signed int lon; /* Longitude */
                        signed int lat; /* Latitude */
                        unsigned int status; /* Status of Signal */
                        unsigned int signal; /* Signal In Service */
                        unsigned int hour; /* UTC hour */
                        unsigned int minute; /* UTC minute */
                        unsigned int nextsignal; /* Expected Next Signal */
                    } dac1fid19;
                    /* IMO289 - Route info (broadcast) */
                    struct route_info dac1fid27;

                    /* IMO289 - Text message (broadcast) */
                    struct {
                        unsigned int linkage;
#define AIS_DAC1FID29_TEXT_MAX	162	/* 920 bits of six-bit, plus NUL */
                        char text[AIS_DAC1FID29_TEXT_MAX];
                    } dac1fid29;

                    /* IMO289 - Meteorological-Hydrological data */
                    struct {
                        bool accuracy; /* position accuracy, <10m if true */
#define DAC1FID31_LATLON_SCALE	1000
                        int lon; /* longitude in minutes * .001 */
#define DAC1FID31_LON_NOT_AVAILABLE	(181*60*DAC1FID31_LATLON_SCALE)
                        int lat; /* longitude in minutes * .001 */
#define DAC1FID31_LAT_NOT_AVAILABLE	(91*60*DAC1FID31_LATLON_SCALE)
                        unsigned int day; /* UTC day */
                        unsigned int hour; /* UTC hour */
                        unsigned int minute; /* UTC minute */
                        unsigned int wspeed; /* average wind speed */
                        unsigned int wgust; /* wind gust */
#define DAC1FID31_WIND_HIGH			126
#define DAC1FID31_WIND_NOT_AVAILABLE		127
                        unsigned int wdir; /* wind direction */
                        unsigned int wgustdir; /* wind gust direction */
#define DAC1FID31_DIR_NOT_AVAILABLE		360
                        int airtemp; /* temperature, units 0.1C */
#define DAC1FID31_AIRTEMP_NOT_AVAILABLE		-1024
#define DAC1FID31_AIRTEMP_DIV			10.0
                        unsigned int humidity; /* relative humidity, % */
#define DAC1FID31_HUMIDITY_NOT_AVAILABLE	101
                        int dewpoint; /* dew point, units 0.1C */
#define DAC1FID31_DEWPOINT_NOT_AVAILABLE	501
#define DAC1FID31_DEWPOINT_DIV		10.0
                        unsigned int pressure; /* air pressure, hpa */
#define DAC1FID31_PRESSURE_NOT_AVAILABLE	511
#define DAC1FID31_PRESSURE_HIGH			402
#define DAC1FID31_PRESSURE_OFFSET		-799
                        unsigned int pressuretend; /* tendency */
#define DAC1FID31_PRESSURETEND_NOT_AVAILABLE	3
                        bool visgreater; /* visibility greater than */
                        unsigned int visibility; /* units 0.1 nautical miles */
#define DAC1FID31_VISIBILITY_NOT_AVAILABLE	127
#define DAC1FID31_VISIBILITY_DIV		10.0
                        int waterlevel; /* cm */
#define DAC1FID31_WATERLEVEL_NOT_AVAILABLE	4001
#define DAC1FID31_WATERLEVEL_OFFSET		1000
#define DAC1FID31_WATERLEVEL_DIV		100.0
                        unsigned int leveltrend; /* water level trend code */
#define DAC1FID31_WATERLEVELTREND_NOT_AVAILABLE	3
                        unsigned int cspeed; /* current speed in deciknots */
#define DAC1FID31_CSPEED_NOT_AVAILABLE		255
#define DAC1FID31_CSPEED_DIV			10.0
                        unsigned int cdir; /* current dir., degrees */
                        unsigned int cspeed2; /* current speed in deciknots */
                        unsigned int cdir2; /* current dir., degrees */
                        unsigned int cdepth2; /* measurement depth, 0.1m */
#define DAC1FID31_CDEPTH_NOT_AVAILABLE		301
#define DAC1FID31_CDEPTH_SCALE			10.0
                        unsigned int cspeed3; /* current speed in deciknots */
                        unsigned int cdir3; /* current dir., degrees */
                        unsigned int cdepth3; /* measurement depth, 0.1m */
                        unsigned int waveheight; /* in decimeters */
#define DAC1FID31_HEIGHT_NOT_AVAILABLE		31
#define DAC1FID31_HEIGHT_DIV			10.0
                        unsigned int waveperiod; /* in seconds */
#define DAC1FID31_PERIOD_NOT_AVAILABLE		63
                        unsigned int wavedir; /* direction in degrees */
                        unsigned int swellheight; /* in decimeters */
                        unsigned int swellperiod; /* in seconds */
                        unsigned int swelldir; /* direction in degrees */
                        unsigned int seastate; /* Beaufort scale, 0-12 */
#define DAC1FID31_SEASTATE_NOT_AVAILABLE	15
                        int watertemp; /* units 0.1deg Celsius */
#define DAC1FID31_WATERTEMP_NOT_AVAILABLE	601
#define DAC1FID31_WATERTEMP_DIV		10.0
                        unsigned int preciptype; /* 0-7, enumerated */
#define DAC1FID31_PRECIPTYPE_NOT_AVAILABLE	7
                        unsigned int salinity; /* units of 0.1 permil (ca. PSU) */
#define DAC1FID31_SALINITY_NOT_AVAILABLE	510
#define DAC1FID31_SALINITY_DIV		10.0
                        unsigned int ice; /* is there sea ice? */
#define DAC1FID31_ICE_NOT_AVAILABLE		3
                    } dac1fid31;
                };
            } type8;

            /* Type 9 - Standard SAR Aircraft Position Report */
            struct {
                unsigned int alt; /* altitude in meters */
#define AIS_ALT_NOT_AVAILABLE	4095
#define AIS_ALT_HIGH    	4094	/* 4094 meters or higher */
                unsigned int speed; /* speed over ground in deciknots */
#define AIS_SAR_SPEED_NOT_AVAILABLE	1023
#define AIS_SAR_FAST_MOVER  	1022
                bool accuracy; /* position accuracy */
                int lon; /* longitude */
                int lat; /* latitude */
                unsigned int course; /* course over ground */
                unsigned int second; /* seconds of UTC timestamp */
                unsigned int regional; /* regional reserved */
                unsigned int dte; /* data terminal enable */
                bool assigned; /* assigned-mode flag */
                bool raim; /* RAIM flag */
                unsigned int radio; /* radio status bits */
            } type9;

            /* Type 10 - UTC/Date Inquiry */
            struct {
                unsigned int dest_mmsi; /* destination MMSI */
            } type10;

            /* Type 12 - Safety-Related Message */
            struct {
                unsigned int seqno; /* sequence number */
                unsigned int dest_mmsi; /* destination MMSI */
                bool retransmit; /* retransmit flag */
#define AIS_TYPE12_TEXT_MAX	157	/* 936 bits of six-bit, plus NUL */
                char text[AIS_TYPE12_TEXT_MAX];
            } type12;

            /* Type 14 - Safety-Related Broadcast Message */
            struct {
#define AIS_TYPE14_TEXT_MAX	161	/* 952 bits of six-bit, plus NUL */
                char text[AIS_TYPE14_TEXT_MAX];
            } type14;

            /* Type 15 - Interrogation */
            struct {
                unsigned int mmsi1;
                unsigned int type1_1;
                unsigned int offset1_1;
                unsigned int type1_2;
                unsigned int offset1_2;
                unsigned int mmsi2;
                unsigned int type2_1;
                unsigned int offset2_1;
            } type15;

            /* Type 16 - Assigned Mode Command */
            struct {
                unsigned int mmsi1;
                unsigned int offset1;
                unsigned int increment1;
                unsigned int mmsi2;
                unsigned int offset2;
                unsigned int increment2;
            } type16;

            /* Type 17 - GNSS Broadcast Binary Message */
            struct {
#define AIS_GNSS_LATLON_DIV	600.0
                int lon; /* longitude */
                int lat; /* latitude */
#define AIS_TYPE17_BINARY_MAX	736	/* 920 bits */
                size_t bitcount; /* bit count of the data */
                char bitdata[(AIS_TYPE17_BINARY_MAX + 7) / 8];
            } type17;

            /* Type 18 - Standard Class B CS Position Report */
            struct {
                unsigned int reserved; /* altitude in meters */
                unsigned int speed; /* speed over ground in deciknots */
                bool accuracy; /* position accuracy */
                int lon; /* longitude */
#define AIS_GNS_LON_NOT_AVAILABLE	0x1a838
                int lat; /* latitude */
#define AIS_GNS_LAT_NOT_AVAILABLE	0xd548
                unsigned int course; /* course over ground */
                unsigned int heading; /* true heading */
                unsigned int second; /* seconds of UTC timestamp */
                unsigned int regional; /* regional reserved */
                bool cs; /* carrier sense unit flag */
                bool display; /* unit has attached display? */
                bool dsc; /* unit attached to radio with DSC? */
                bool band; /* unit can switch frequency bands? */
                bool msg22; /* can accept Message 22 management? */
                bool assigned; /* assigned-mode flag */
                bool raim; /* RAIM flag */
                unsigned int radio; /* radio status bits */
            } type18;

            /* Type 19 - Extended Class B CS Position Report */
            struct {
                unsigned int reserved; /* altitude in meters */
                unsigned int speed; /* speed over ground in deciknots */
                bool accuracy; /* position accuracy */
                int lon; /* longitude */
                int lat; /* latitude */
                unsigned int course; /* course over ground */
                unsigned int heading; /* true heading */
                unsigned int second; /* seconds of UTC timestamp */
                unsigned int regional; /* regional reserved */
                char shipname[AIS_SHIPNAME_MAXLEN + 1]; /* ship name */
                unsigned int shiptype; /* ship type code */
                unsigned int to_bow; /* dimension to bow */
                unsigned int to_stern; /* dimension to stern */
                unsigned int to_port; /* dimension to port */
                unsigned int to_starboard; /* dimension to starboard */
                unsigned int epfd; /* type of position fix deviuce */
                bool raim; /* RAIM flag */
                unsigned int dte; /* date terminal enable */
                bool assigned; /* assigned-mode flag */
            } type19;

            /* Type 20 - Data Link Management Message */
            struct {
                unsigned int offset1; /* TDMA slot offset */
                unsigned int number1; /* number of xlots to allocate */
                unsigned int timeout1; /* allocation timeout */
                unsigned int increment1; /* repeat increment */
                unsigned int offset2; /* TDMA slot offset */
                unsigned int number2; /* number of xlots to allocate */
                unsigned int timeout2; /* allocation timeout */
                unsigned int increment2; /* repeat increment */
                unsigned int offset3; /* TDMA slot offset */
                unsigned int number3; /* number of xlots to allocate */
                unsigned int timeout3; /* allocation timeout */
                unsigned int increment3; /* repeat increment */
                unsigned int offset4; /* TDMA slot offset */
                unsigned int number4; /* number of xlots to allocate */
                unsigned int timeout4; /* allocation timeout */
                unsigned int increment4; /* repeat increment */
            } type20;

            /* Type 21 - Aids to Navigation Report */
            struct {
                unsigned int aid_type; /* aid type */
                char name[35]; /* name of aid to navigation */
                bool accuracy; /* position accuracy */
                int lon; /* longitude */
                int lat; /* latitude */
                unsigned int to_bow; /* dimension to bow */
                unsigned int to_stern; /* dimension to stern */
                unsigned int to_port; /* dimension to port */
                unsigned int to_starboard; /* dimension to starboard */
                unsigned int epfd; /* type of EPFD */
                unsigned int second; /* second of UTC timestamp */
                bool off_position; /* off-position indicator */
                unsigned int regional; /* regional reserved field */
                bool raim; /* RAIM flag */
                bool virtual_aid; /* is virtual station? */
                bool assigned; /* assigned-mode flag */
            } type21;

            /* Type 22 - Channel Management */
            struct {
                unsigned int channel_a; /* Channel A number */
                unsigned int channel_b; /* Channel B number */
                unsigned int txrx; /* transmit/receive mode */
                bool power; /* high-power flag */
#define AIS_CHANNEL_LATLON_DIV	600.0

                union {

                    struct {
                        int ne_lon; /* NE corner longitude */
                        int ne_lat; /* NE corner latitude */
                        int sw_lon; /* SW corner longitude */
                        int sw_lat; /* SW corner latitude */
                    } area;

                    struct {
                        unsigned int dest1; /* addressed station MMSI 1 */
                        unsigned int dest2; /* addressed station MMSI 2 */
                    } mmsi;
                };
                bool addressed; /* addressed vs. broadast flag */
                bool band_a; /* fix 1.5kHz band for channel A */
                bool band_b; /* fix 1.5kHz band for channel B */
                unsigned int zonesize; /* size of transitional zone */
            } type22;

            /* Type 23 - Group Assignment Command */
            struct {
                int ne_lon; /* NE corner longitude */
                int ne_lat; /* NE corner latitude */
                int sw_lon; /* SW corner longitude */
                int sw_lat; /* SW corner latitude */
                unsigned int stationtype; /* station type code */
                unsigned int shiptype; /* ship type code */
                unsigned int txrx; /* transmit-enable code */
                unsigned int interval; /* report interval */
                unsigned int quiet; /* quiet time */
            } type23;

            /* Type 24 - Class B CS Static Data Report */
            struct {
                char shipname[AIS_SHIPNAME_MAXLEN + 1]; /* vessel name */
                unsigned int shiptype; /* ship type code */
                char vendorid[8]; /* vendor ID */
                char callsign[8]; /* callsign */

                union {
                    unsigned int mothership_mmsi; /* MMSI of main vessel */

                    struct {
                        unsigned int to_bow; /* dimension to bow */
                        unsigned int to_stern; /* dimension to stern */
                        unsigned int to_port; /* dimension to port */
                        unsigned int to_starboard; /* dimension to starboard */
                    } dim;
                };
            } type24;

            /* Type 25 - Addressed Binary Message */
            struct {
                bool addressed; /* addressed-vs.broadcast flag */
                bool structured; /* structured-binary flag */
                unsigned int dest_mmsi; /* destination MMSI */
                unsigned int app_id; /* Application ID */
#define AIS_TYPE25_BINARY_MAX	128	/* Up to 128 bits */
                size_t bitcount; /* bit count of the data */
                char bitdata[(AIS_TYPE25_BINARY_MAX + 7) / 8];
            } type25;

            /* Type 26 - Addressed Binary Message */
            struct {
                bool addressed; /* addressed-vs.broadcast flag */
                bool structured; /* structured-binary flag */
                unsigned int dest_mmsi; /* destination MMSI */
                unsigned int app_id; /* Application ID */
#define AIS_TYPE26_BINARY_MAX	1004	/* Up to 128 bits */
                size_t bitcount; /* bit count of the data */
                char bitdata[(AIS_TYPE26_BINARY_MAX + 7) / 8];
                unsigned int radio; /* radio status bits */
            } type26;

            /* Type 27 - Long Range AIS Broadcast message */
            struct {
                bool accuracy; /* position accuracy */
                bool raim; /* RAIM flag */
                unsigned int status; /* navigation status */
#define AIS_LONGRANGE_LATLON_DIV	600.0
                int lon; /* longitude */
#define AIS_LONGRANGE_LON_NOT_AVAILABLE	0x1a838
                int lat; /* latitude */
#define AIS_LONGRANGE_LAT_NOT_AVAILABLE	0xd548
                unsigned int speed; /* speed over ground in deciknots */
#define AIS_LONGRANGE_SPEED_NOT_AVAILABLE 63
                unsigned int course; /* course over ground */
#define AIS_LONGRANGE_COURSE_NOT_AVAILABLE 511
                bool gnss; /* are we reporting GNSS position? */
            } type27;
        };
    };

    /* state for resolving interleaved Type 24 packets */
    struct ais_type24a_t {
        unsigned int mmsi;
        char shipname[AIS_SHIPNAME_MAXLEN + 1];
    };
#define MAX_TYPE24_INTERLEAVE	8	/* max number of queued type 24s */

    struct ais_type24_queue_t {
        struct ais_type24a_t ships[MAX_TYPE24_INTERLEAVE];
        int index;
    };

    /* state for resolving AIVDM decodes */
    struct aivdm_context_t {
        /* hold context for decoding AIDVM packet sequences */
        int decoded_frags; /* for tracking AIDVM parts in a multipart sequence */
        unsigned char bits[2048];
        size_t bitlen; /* how many valid bits */
        struct ais_type24_queue_t type24_queue;
    };

#define MODE_NMEA	0
#define MODE_BINARY	1


    /* logging levels */
#define LOG_ERROR 	-1	/* errors, display always */
#define LOG_SHOUT	0	/* not an error but we should always see it */
#define LOG_WARN	1	/* not errors but may indicate a problem */
#define LOG_INF 	2	/* key informative messages */
#define LOG_DATA	3	/* log data management messages */
#define LOG_PROG	4	/* progress messages */
#define LOG_IO  	5	/* IO to and from devices */
#define LOG_SPIN	6	/* logging for catching spin bugs */
#define LOG_RAW 	7	/* raw low-level I/O */

#define GPS_PATH_MAX	128	/* dev files usually have short names */

#define NITEMS(x) (int)(sizeof(x)/sizeof(x[0]))


    /* Needed because 4.x versions of GCC are really annoying */
#define ignore_return(funcall)	assert(funcall != -23)

    /* memory barriers */
    static /*@unused@*/ inline void barrier(void) {
#if defined(__GNUC__) && defined(__x86_64__)
        asm volatile("mfence");
#endif /* defined(__GNUC__) && defined(__x86_64__) */
    }


    bool aivdm_decode(const char *buf, size_t buflen,
            struct aivdm_context_t ais_contexts[AIVDM_CHANNELS],
            struct ais_t *ais);

#include <stdbool.h>
#include <ctype.h>

    typedef enum {
        t_integer, t_uinteger, t_real,
        t_string, t_boolean, t_character,
        t_time,
        t_object, t_structobject, t_array,
        t_check
    } json_type;

    struct json_enum_t {
        char *name;
        int value;
    };

    struct json_array_t {
        json_type element_type;

        union {

            struct {
                const struct json_attr_t *subtype;
                char *base;
                size_t stride;
            } objects;

            struct {
                char **ptrs;
                char *store;
                int storelen;
            } strings;
        } arr;
        int *count, maxlen;
    };

    struct json_attr_t {
        char *attribute;
        json_type type;

        union {
            int *integer;
            unsigned int *uinteger;
            double *real;
            char *string;
            bool *boolean;
            char *character;
            struct json_array_t array;
            size_t offset;
        } addr;

        union {
            int integer;
            unsigned int uinteger;
            double real;
            bool boolean;
            char character;
            char *check;
        } dflt;
        size_t len;
        const struct json_enum_t *map;
        bool nodefault;
    };

#define JSON_ATTR_MAX	31	/* max chars in JSON attribute name */
#define JSON_VAL_MAX	512	/* max chars in JSON value part */

#ifdef __cplusplus
    extern "C" {
#endif
        int json_read_object(const char *, const struct json_attr_t *,
                /*@null@*/const char **);
        int json_read_array(const char *, const struct json_array_t *,
                /*@null@*/const char **);
        const /*@observer@*/char *json_error_string(int);

        void json_enable_debug(int, FILE *);
#ifdef __cplusplus
    }
#endif

#define JSON_ERR_OBSTART	1	/* non-WS when expecting object start */
#define JSON_ERR_ATTRSTART	2	/* non-WS when expecting attrib start */
#define JSON_ERR_BADATTR	3	/* unknown attribute name */
#define JSON_ERR_ATTRLEN	4	/* attribute name too long */
#define JSON_ERR_NOARRAY	5	/* saw [ when not expecting array */
#define JSON_ERR_NOBRAK 	6	/* array element specified, but no [ */
#define JSON_ERR_STRLONG	7	/* string value too long */
#define JSON_ERR_TOKLONG	8	/* token value too long */
#define JSON_ERR_BADTRAIL	9	/* garbage while expecting , or } */
#define JSON_ERR_ARRAYSTART	10	/* didn't find expected array start */
#define JSON_ERR_OBJARR 	11	/* error while parsing object array */
#define JSON_ERR_SUBTOOLONG	12	/* too many array elements */
#define JSON_ERR_BADSUBTRAIL	13	/* garbage while expecting array comma */
#define JSON_ERR_SUBTYPE	14	/* unsupported array element type */
#define JSON_ERR_BADSTRING	15	/* error while string parsing */
#define JSON_ERR_CHECKFAIL	16	/* check attribute not matched */
#define JSON_ERR_NOPARSTR	17	/* can't support strings in parallel arrays */
#define JSON_ERR_BADENUM	18	/* invalid enumerated value */
#define JSON_ERR_QNONSTRING	19	/* saw quoted value when expecting nonstring */
#define JSON_ERR_NONQSTRING	19	/* didn't see quoted value when expecting string */
#define JSON_ERR_MISC		20	/* other data conversion error */
#define JSON_ERR_BADNUM		21	/* error while parsing a numerical argument */
#define JSON_ERR_NULLPTR	22	/* unexpected null value or attribute pointer */

    /*
     * Use the following macros to declare template initializers for structobject
     * arrays.  Writing the equivalents out by hand is error-prone.
     *
     * STRUCTOBJECT takes a structure name s, and a fieldname f in s.
     *
     * STRUCTARRAY takes the name of a structure array, a pointer to a an
     * initializer defining the subobject type, and the address of an integer to
     * store the length in.
     */
#define STRUCTOBJECT(s, f)	.addr.offset = offsetof(s, f)
#define STRUCTARRAY(a, e, n) \
	.addr.array.element_type = t_structobject, \
	.addr.array.arr.objects.subtype = e, \
	.addr.array.arr.objects.base = (char*)a, \
	.addr.array.arr.objects.stride = sizeof(a[0]), \
	.addr.array.count = n, \
	.addr.array.maxlen = NITEMS(a)

    char *json_stringify(char *, size_t, const char *);

    void json_version_dump(char *, size_t);
    void json_aivdm_dump(const struct ais_t *, const char *, bool, char *, size_t);

    int json_ais_read(const char *, char *, size_t, struct ais_t *, const char **);

#define GPS_JSON_COMMAND_MAX        80
#define GPS_JSON_RESPONSE_MAX       4096
    
#define SAD_INIT   0     
#define SAD_GATHER 1
#define SAD_DECODE 1    
    
    typedef struct sad_frag_ctx_s {
        uint8_t seq_len;
        uint8_t seq_cur;
        uint8_t seq_id;
        char seq_chan;
    } sad_frag_ctx_t;

    typedef struct sad_filter_s {
        struct ais_t ais;
        int (*f_ais_cb)(struct sad_filter_s*);
        void* userdata;
        sub0_substring_t* sentence;
        uint8_t state;
        sad_frag_ctx_t frag_cur;
        sad_frag_ctx_t frag_prev;
        uint64_t sentences;
        uint64_t frags;
        uint64_t errors;
        uint64_t duplicates;
        uint64_t types[AIVDM_MESSAGES_TYPE + 1];
        char last_sentence[1024];
        char forward_sentence[1024];

    } sad_filter_t;

    int sad_filter_init(sad_filter_t* f_, int (*f_ais_cb)(struct sad_filter_s*), void* userdata_);

    int sad_stats_string(char **, sad_filter_t*);
    int sad_decode_file(sad_filter_t* filter_, const char* filename_);
    int sad_decode_multiline(sad_filter_t* filter_, const char* buffer_, size_t n_);

#ifdef __cplusplus
}
#endif

#endif /* _AISD_H_ */

