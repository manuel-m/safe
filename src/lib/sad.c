#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <stdarg.h>

#include "sad.h"

#ifndef HAVE_STRLCAT

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t strlcat(char *dst, const char *src, size_t siz) {
    size_t slen = strlen(src);
    size_t dlen = strlen(dst);
    if (siz != 0) {
        if (dlen + slen < siz)
            memcpy(dst + dlen, src, slen + 1);
        else {
            memcpy(dst + dlen, src, siz - dlen - 1);
            dst[siz - 1] = '\0';
        }
    }
    return dlen + slen;
}

#endif /* HAVE_STRLCAT */

#ifndef HAVE_STRLCPY

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t siz) {
    size_t len = strlen(src);
    if (siz != 0) {
        if (len >= siz) {
            memcpy(dst, src, siz - 1);
            dst[siz - 1] = '\0';
        } else
            memcpy(dst, src, len + 1);
    }
    return len;
}

#endif /* HAVE_STRLCPY */

#if defined(_WIN32)
#include "mmwincompat.h"
#endif

const char *gpsd_hexdump(char *binbuf, size_t binbuflen) {
    /* FIXME: this isn't thead-safe! */
    static char hexbuf[MAX_PACKET_LENGTH * 2 + 1];
    size_t i, j = 0;
    size_t len =
            (size_t) ((binbuflen >
            MAX_PACKET_LENGTH) ? MAX_PACKET_LENGTH : binbuflen);
    const char *ibuf = (const char *) binbuf;
    const char *hexchar = "0123456789abcdef";

    if (NULL == binbuf || 0 == binbuflen)
        return "";

    for (i = 0; i < len; i++) {
        hexbuf[j++] = hexchar[(ibuf[i] & 0xf0) >> 4];
        hexbuf[j++] = hexchar[ibuf[i] & 0x0f];
    }
    hexbuf[j] = '\0';
    return hexbuf;
}

uint64_t ubits(unsigned char buf[], unsigned int start, unsigned int width)
/* extract a (zero-origin) bitfield from the buffer as an unsigned big-endian uint64_t */ {
    uint64_t fld = 0;
    unsigned int i;
    unsigned end;

    assert(width <= sizeof (uint64_t) * CHAR_BIT);
    for (i = start / CHAR_BIT;
            i < (start + width + CHAR_BIT - 1) / CHAR_BIT; i++) {
        fld <<= CHAR_BIT;
        fld |= (unsigned char) buf[i];
    }

    end = (start + width) % CHAR_BIT;
    if (end != 0) {
        fld >>= (CHAR_BIT - end);
    }

    fld &= ~(-1LL << width);
    return fld;
}

int64_t sbits(signed char buf[], unsigned int start, unsigned int width)
/* extract a bitfield from the buffer as a signed big-endian long */ {
    uint64_t fld = ubits((unsigned char *) buf, start, width);

    if (fld & (1LL << (width - 1))) {
        fld |= (-1LL << (width - 1));
    }
    return (int64_t) fld;
}



/*
 * Parse the data from the device
 */

#ifdef SAD_ENABLE_MESSAGE_5
# define SAD_ENABLE_MESSAGE_5_OR_19
#endif

#ifdef SAD_ENABLE_MESSAGE_19
#define SAD_ENABLE_MESSAGE_5_OR_19
#endif


#ifdef SAD_ENABLE_MESSAGE_5_OR_19
static void from_sixbit(unsigned char *bitvec, unsigned int start, int count, char *to) {

    const char sixchr[64] =
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^- !\"#$%&'()*+,-./0123456789:;<=>?";

    int i;
    char newchar;

    /* six-bit to ASCII */
    for (i = 0; i < count - 1; i++) {
        newchar = sixchr[ubits(bitvec, start + 6 * i, 6U)];
        if (newchar == '@')
            break;
        else
            to[i] = newchar;
    }
    to[i] = '\0';
    /* trim spaces on right end */
    for (i = count - 2; i >= 0; i--)
        if (to[i] == ' ' || to[i] == '@')
            to[i] = '\0';
        else
            break;
    /*@ -type @*/
}
#endif /* SAD_ENABLE_MESSAGE_5_OR_19 */

char *json_stringify(/*@out@*/ char *to,
        size_t len,
        /*@in@*/ const char *from)
/* escape double quotes and control characters inside a JSON string */ {
    /*@-temptrans@*/
    const char *sp;
    char *tp;

    tp = to;
    /*
     * The limit is len-6 here because we need to be leave room for
     * each character to generate an up to 6-character Java-style
     * escape
     */
    for (sp = from; *sp != '\0' && ((tp - to) < ((int) len - 6)); sp++) {
        if (!isascii((unsigned char) *sp) || iscntrl((unsigned char) *sp)) {
            *tp++ = '\\';
            switch (*sp) {
                case '\b':
                    *tp++ = 'b';
                    break;
                case '\f':
                    *tp++ = 'f';
                    break;
                case '\n':
                    *tp++ = 'n';
                    break;
                case '\r':
                    *tp++ = 'r';
                    break;
                case '\t':
                    *tp++ = 't';
                    break;
                default:
                    /* ugh, we'd prefer a C-style escape here, but this is JSON */
                    /* http://www.ietf.org/rfc/rfc4627.txt
                     * section 2.5, escape is \uXXXX */
                    /* don't forget the NUL in the output count! */
                    (void) snprintf(tp, 6, "u%04x", 0x00ff & (unsigned int) *sp);
                    tp += strlen(tp);
            }
        } else {
            if (*sp == '"' || *sp == '\\')
                *tp++ = '\\';
            *tp++ = *sp;
        }
    }
    *tp = '\0';

    return to;
    /*@+temptrans@*/
}


#define JSON_BOOL(x)	((x)?"true":"false")

void json_aivdm_dump(const struct ais_t *ais,
        /*@null@*/const char *device, bool scaled_,
        /*@out@*/char *buf, size_t buflen) {
//     char buf1[JSON_VAL_MAX * 2 + 1];

    

    static char *nav_legends[] = {
        "Under way using engine",
        "At anchor",
        "Not under command",
        "Restricted manoeuverability",
        "Constrained by her draught",
        "Moored",
        "Aground",
        "Engaged in fishing",
        "Under way sailing",
        "Reserved for HSC",
        "Reserved for WIG",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Not defined",
    };


#ifdef SAD_ENABLE_MESSAGE_5_OR_19
    
    static char *epfd_legends[] = {
        "Undefined",
        "GPS",
        "GLONASS",
        "Combined GPS/GLONASS",
        "Loran-C",
        "Chayka",
        "Integrated navigation system",
        "Surveyed",
        "Galileo",
    };

#define EPFD_DISPLAY(n) (((n) < (unsigned int)NITEMS(epfd_legends)) ? epfd_legends[n] : "INVALID EPFD")
    
    
    
    static char *ship_type_legends[100] = {
        "Not available",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Reserved for future use",
        "Wing in ground (WIG) - all ships of this type",
        "Wing in ground (WIG) - Hazardous category A",
        "Wing in ground (WIG) - Hazardous category B",
        "Wing in ground (WIG) - Hazardous category C",
        "Wing in ground (WIG) - Hazardous category D",
        "Wing in ground (WIG) - Reserved for future use",
        "Wing in ground (WIG) - Reserved for future use",
        "Wing in ground (WIG) - Reserved for future use",
        "Wing in ground (WIG) - Reserved for future use",
        "Wing in ground (WIG) - Reserved for future use",
        "Fishing",
        "Towing",
        "Towing: length exceeds 200m or breadth exceeds 25m",
        "Dredging or underwater ops",
        "Diving ops",
        "Military ops",
        "Sailing",
        "Pleasure Craft",
        "Reserved",
        "Reserved",
        "High speed craft (HSC) - all ships of this type",
        "High speed craft (HSC) - Hazardous category A",
        "High speed craft (HSC) - Hazardous category B",
        "High speed craft (HSC) - Hazardous category C",
        "High speed craft (HSC) - Hazardous category D",
        "High speed craft (HSC) - Reserved for future use",
        "High speed craft (HSC) - Reserved for future use",
        "High speed craft (HSC) - Reserved for future use",
        "High speed craft (HSC) - Reserved for future use",
        "High speed craft (HSC) - No additional information",
        "Pilot Vessel",
        "Search and Rescue vessel",
        "Tug",
        "Port Tender",
        "Anti-pollution equipment",
        "Law Enforcement",
        "Spare - Local Vessel",
        "Spare - Local Vessel",
        "Medical Transport",
        "Ship according to RR Resolution No. 18",
        "Passenger - all ships of this type",
        "Passenger - Hazardous category A",
        "Passenger - Hazardous category B",
        "Passenger - Hazardous category C",
        "Passenger - Hazardous category D",
        "Passenger - Reserved for future use",
        "Passenger - Reserved for future use",
        "Passenger - Reserved for future use",
        "Passenger - Reserved for future use",
        "Passenger - No additional information",
        "Cargo - all ships of this type",
        "Cargo - Hazardous category A",
        "Cargo - Hazardous category B",
        "Cargo - Hazardous category C",
        "Cargo - Hazardous category D",
        "Cargo - Reserved for future use",
        "Cargo - Reserved for future use",
        "Cargo - Reserved for future use",
        "Cargo - Reserved for future use",
        "Cargo - No additional information",
        "Tanker - all ships of this type",
        "Tanker - Hazardous category A",
        "Tanker - Hazardous category B",
        "Tanker - Hazardous category C",
        "Tanker - Hazardous category D",
        "Tanker - Reserved for future use",
        "Tanker - Reserved for future use",
        "Tanker - Reserved for future use",
        "Tanker - Reserved for future use",
        "Tanker - No additional information",
        "Other Type - all ships of this type",
        "Other Type - Hazardous category A",
        "Other Type - Hazardous category B",
        "Other Type - Hazardous category C",
        "Other Type - Hazardous category D",
        "Other Type - Reserved for future use",
        "Other Type - Reserved for future use",
        "Other Type - Reserved for future use",
        "Other Type - Reserved for future use",
        "Other Type - no additional information",
    };

#define SHIPTYPE_DISPLAY(n) (((n) < (unsigned int)NITEMS(ship_type_legends)) ? ship_type_legends[n] : "INVALID SHIP TYPE")
#endif /* SAD_ENABLE_MESSAGE_5_OR_19 */
    

#define STATIONTYPE_DISPLAY(n) (((n) < (unsigned int)NITEMS(station_type_legends)) ? station_type_legends[n] : "INVALID STATION TYPE")


#define NAVAIDTYPE_DISPLAY(n) (((n) < (unsigned int)NITEMS(navaid_type_legends[0])) ? navaid_type_legends[n] : "INVALID NAVAID TYPE")

#ifdef SAD_ENABLE_MESSAGE_8        
    static const char *signal_legends[] = {
        "N/A",
        "Serious emergency – stop or divert according to instructions.",
        "Vessels shall not proceed.",
        "Vessels may proceed. One way traffic.",
        "Vessels may proceed. Two way traffic.",
        "Vessels shall proceed on specific orders only.",
        "Vessels in main channel shall not proceed."
        "Vessels in main channel shall proceed on specific orders only.",
        "Vessels in main channel shall proceed on specific orders only.",
        "I = \"in-bound\" only acceptable.",
        "O = \"out-bound\" only acceptable.",
        "F = both \"in- and out-bound\" acceptable.",
        "XI = Code will shift to \"I\" in due time.",
        "XO = Code will shift to \"O\" in due time.",
        "X = Vessels shall proceed only on direction.",
    };

#define SIGNAL_DISPLAY(n) (((n) < (unsigned int)NITEMS(signal_legends[0])) ? signal_legends[n] : "INVALID SIGNAL TYPE")

    static const char *route_type[32] = {
        "Undefined (default)",
        "Mandatory",
        "Recommended",
        "Alternative",
        "Recommended route through ice",
        "Ship route plan",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Reserved for future use.",
        "Cancel route identified by message linkage",
    };


    static char *idtypes[] = {
        "mmsi",
        "imo",
        "callsign",
        "other",
    };
#endif /* SAD_ENABLE_MESSAGE_8 */    
    

#ifdef SAD_ENABLE_MESSAGE_6
    static const char *racon_status[] = {
        "No RACON installed",
        "RACON not monitored",
        "RACON operational",
        "RACON ERROR"
    };
    static const char *light_status[] = {
        "No light or no monitoring",
        "Light ON",
        "Light OFF",
        "Light ERROR"
    };
#endif /* SAD_ENABLE_MESSAGE_6 */

    (void) snprintf(buf, buflen, "{\"class\":\"AIS\",");
    if (device != NULL && device[0] != '\0')
        (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
            "\"device\":\"%s\",", device);
    (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
            "\"type\":%u,\"repeat\":%u,\"mmsi\":%u,\"scaled\":%s,",
            ais->type, ais->repeat, ais->mmsi, JSON_BOOL(scaled_));
    /*@ -formatcode -mustfreefresh @*/
    switch (ais->type) {     
        case 1: /* Position Report */
        case 2:
        case 3:
            if (scaled_) {
                char turnlegend[20];
                char speedlegend[20];

                /*
                 * Express turn as nan if not available,
                 * "fastleft"/"fastright" for fast turns.
                 */
                if (ais->type1.turn == -128)
                    (void)strlcpy(turnlegend, "\"nan\"", sizeof (turnlegend));
                else if (ais->type1.turn == -127)
                    (void)strlcpy(turnlegend, "\"fastleft\"", sizeof (turnlegend));
                else if (ais->type1.turn == 127)
                    (void)strlcpy(turnlegend, "\"fastright\"",
                        sizeof (turnlegend));
                else {
                    double rot1 = ais->type1.turn / 4.733;
                    (void) snprintf(turnlegend, sizeof (turnlegend),
                            "%.0f", rot1 * rot1);
                }

                /*
                 * Express speed as nan if not available,
                 * "fast" for fast movers.
                 */
                if (ais->type1.speed == AIS_SPEED_NOT_AVAILABLE)
                    (void)strlcpy(speedlegend, "\"nan\"", sizeof (speedlegend));
                else if (ais->type1.speed == AIS_SPEED_FAST_MOVER)
                    (void)strlcpy(speedlegend, "\"fast\"", sizeof (speedlegend));
                else
                    (void) snprintf(speedlegend, sizeof (speedlegend),
                        "%.1f", ais->type1.speed / 10.0);

                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                        "\"status\":\"%s\",\"turn\":%s,\"speed\":%s,"
                        "\"accuracy\":%s,\"lon\":%.4f,\"lat\":%.4f,"
                        "\"course\":%.1f,\"heading\":%u,\"second\":%u,"
                        "\"maneuver\":%u,\"raim\":%s,\"radio\":%u}\r\n",
                        nav_legends[ais->type1.status],
                        turnlegend,
                        speedlegend,
                        JSON_BOOL(ais->type1.accuracy),
                        ais->type1.lon / AIS_LATLON_DIV,
                        ais->type1.lat / AIS_LATLON_DIV,
                        ais->type1.course / 10.0,
                        ais->type1.heading,
                        ais->type1.second,
                        ais->type1.maneuver,
                        JSON_BOOL(ais->type1.raim), ais->type1.radio);
            } else {
                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                        "\"status\":%u,\"turn\":%d,\"speed\":%u,"
                        "\"accuracy\":%s,\"lon\":%d,\"lat\":%d,"
                        "\"course\":%u,\"heading\":%u,\"second\":%u,"
                        "\"maneuver\":%u,\"raim\":%s,\"radio\":%u}\r\n",
                        ais->type1.status,
                        ais->type1.turn,
                        ais->type1.speed,
                        JSON_BOOL(ais->type1.accuracy),
                        ais->type1.lon,
                        ais->type1.lat,
                        ais->type1.course,
                        ais->type1.heading,
                        ais->type1.second,
                        ais->type1.maneuver,
                        JSON_BOOL(ais->type1.raim), ais->type1.radio);
            }
            break;
#ifdef SAD_ENABLE_MESSAGE_5            
        case 5: /* Ship static and voyage related data */
        {
              char buf1[JSON_VAL_MAX * 2 + 1];
              char buf2[JSON_VAL_MAX * 2 + 1];
              char buf3[JSON_VAL_MAX * 2 + 1];
            /* some fields have beem merged to an ISO8601 partial date */
            if (scaled_) {
                /* *INDENT-OFF* */
                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                        "\"imo\":%u,\"ais_version\":%u,\"callsign\":\"%s\","
                        "\"shipname\":\"%s\",\"shiptype\":\"%s\","
                        "\"to_bow\":%u,\"to_stern\":%u,\"to_port\":%u,"
                        "\"to_starboard\":%u,\"epfd\":\"%s\","
                        "\"eta\":\"%02u-%02uT%02u:%02uZ\","
                        "\"draught\":%.1f,\"destination\":\"%s\","
                        "\"dte\":%u}\r\n",
                        ais->type5.imo,
                        ais->type5.ais_version,
                        json_stringify(buf1, sizeof (buf1),
                        ais->type5.callsign),
                        json_stringify(buf2, sizeof (buf2),
                        ais->type5.shipname),
                        SHIPTYPE_DISPLAY(ais->type5.shiptype),
                        ais->type5.to_bow, ais->type5.to_stern,
                        ais->type5.to_port, ais->type5.to_starboard,
                        EPFD_DISPLAY(ais->type5.epfd), ais->type5.month,
                        ais->type5.day, ais->type5.hour, ais->type5.minute,
                        ais->type5.draught / 10.0,
                        json_stringify(buf3, sizeof (buf3),
                        ais->type5.destination),
                        ais->type5.dte);
                /* *INDENT-ON* */
            } else {
                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                        "\"imo\":%u,\"ais_version\":%u,\"callsign\":\"%s\","
                        "\"shipname\":\"%s\",\"shiptype\":%u,"
                        "\"to_bow\":%u,\"to_stern\":%u,\"to_port\":%u,"
                        "\"to_starboard\":%u,\"epfd\":%u,"
                        "\"eta\":\"%02u-%02uT%02u:%02uZ\","
                        "\"draught\":%u,\"destination\":\"%s\","
                        "\"dte\":%u}\r\n",
                        ais->type5.imo,
                        ais->type5.ais_version,
                        json_stringify(buf1, sizeof (buf1),
                        ais->type5.callsign),
                        json_stringify(buf2, sizeof (buf2),
                        ais->type5.shipname),
                        ais->type5.shiptype, ais->type5.to_bow,
                        ais->type5.to_stern, ais->type5.to_port,
                        ais->type5.to_starboard, ais->type5.epfd,
                        ais->type5.month, ais->type5.day, ais->type5.hour,
                        ais->type5.minute, ais->type5.draught,
                        json_stringify(buf3, sizeof (buf3),
                        ais->type5.destination),
                        ais->type5.dte);
            }
        }
            break;
        
#endif /* SAD_ENABLE_MESSAGE_5 */
#ifdef SAD_ENABLE_MESSAGE_6
        case 6: /* Binary Message */
        {
          char buf1[JSON_VAL_MAX * 2 + 1];
          char buf2[JSON_VAL_MAX * 2 + 1];
          char buf3[JSON_VAL_MAX * 2 + 1];
          char buf4[JSON_VAL_MAX * 2 + 1];
          int imo = false;
          int i;
            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                    "\"seqno\":%u,\"dest_mmsi\":%u,"
                    "\"retransmit\":%s,\"dac\":%u,\"fid\":%u,",
                    ais->type6.seqno,
                    ais->type6.dest_mmsi,
                    JSON_BOOL(ais->type6.retransmit),
                    ais->type6.dac,
                    ais->type6.fid);
            if (ais->type6.dac == 235 || ais->type6.dac == 250) {
                switch (ais->type6.fid) {
                    case 10: /* GLA - AtoN monitoring data */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"off_pos\":%s,\"alarm\":%s,"
                                "\"stat_ext\":%u,",
                                JSON_BOOL(ais->type6.dac235fid10.off_pos),
                                JSON_BOOL(ais->type6.dac235fid10.alarm),
                                ais->type6.dac235fid10.stat_ext);
                        if (scaled_ && ais->type6.dac235fid10.ana_int != 0)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"ana_int\":%.2f,",
                                ais->type6.dac235fid10.ana_int * 0.05);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"ana_int\":%u,",
                                ais->type6.dac235fid10.ana_int);
                        if (scaled_ && ais->type6.dac235fid10.ana_ext1 != 0)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"ana_ext1\":%.2f,",
                                ais->type6.dac235fid10.ana_ext1 * 0.05);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"ana_ext1\":%u,",
                                ais->type6.dac235fid10.ana_ext1);
                        if (scaled_ && ais->type6.dac235fid10.ana_ext2 != 0)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"ana_ext2\":%.2f,",
                                ais->type6.dac235fid10.ana_ext2 * 0.05);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"ana_ext2\":%u,",
                                ais->type6.dac235fid10.ana_ext2);

                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"racon\":\"%s\",\"light\":\"%s\"",
                                racon_status[ais->type6.dac235fid10.racon],
                                light_status[ais->type6.dac235fid10.light]);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"racon\":%u,\"light\":%u",
                                ais->type6.dac235fid10.racon,
                                ais->type6.dac235fid10.light);
                        if (buf[strlen(buf) - 1] == ',')
                            buf[strlen(buf) - 1] = '\0';
                        (void) strlcat(buf, "}\r\n", buflen);
                        imo = true;
                        break;
                }
            } else if (ais->type6.dac == 1)
                switch (ais->type6.fid) {
                    case 12: /* IMO236 -Dangerous cargo indication */
                        /* some fields have beem merged to an ISO8601 partial date */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"lastport\":\"%s\",\"departure\":\"%02u-%02uT%02u:%02uZ\","
                                "\"nextport\":\"%s\",\"eta\":\"%02u-%02uT%02u:%02uZ\","
                                "\"dangerous\":\"%s\",\"imdcat\":\"%s\","
                                "\"unid\":%u,\"amount\":%u,\"unit\":%u}\r\n",
                                json_stringify(buf1, sizeof (buf1),
                                ais->type6.dac1fid12.lastport),
                                ais->type6.dac1fid12.lmonth,
                                ais->type6.dac1fid12.lday,
                                ais->type6.dac1fid12.lhour,
                                ais->type6.dac1fid12.lminute,
                                json_stringify(buf2, sizeof (buf2),
                                ais->type6.dac1fid12.nextport),
                                ais->type6.dac1fid12.nmonth,
                                ais->type6.dac1fid12.nday,
                                ais->type6.dac1fid12.nhour,
                                ais->type6.dac1fid12.nminute,
                                json_stringify(buf3, sizeof (buf3),
                                ais->type6.dac1fid12.dangerous),
                                json_stringify(buf4, sizeof (buf4),
                                ais->type6.dac1fid12.imdcat),
                                ais->type6.dac1fid12.unid,
                                ais->type6.dac1fid12.amount,
                                ais->type6.dac1fid12.unit);
                        imo = true;
                        break;
                    case 15: /* IMO236 - Extended Ship Static and Voyage Related Data */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"airdraught\":%u}\r\n",
                                ais->type6.dac1fid15.airdraught);
                        imo = true;
                        break;
                    case 16: /* IMO236 - Number of persons on board */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"persons\":%u}\t\n", ais->type6.dac1fid16.persons);
                        imo = true;
                        break;
                    case 18: /* IMO289 - Clearance time to enter port */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"linkage\":%u,\"arrival\":\"%02u-%02uT%02u:%02uZ\",\"portname\":\"%s\",\"destination\":\"%s\",",
                                ais->type6.dac1fid18.linkage,
                                ais->type6.dac1fid18.month,
                                ais->type6.dac1fid18.day,
                                ais->type6.dac1fid18.hour,
                                ais->type6.dac1fid18.minute,
                                json_stringify(buf1, sizeof (buf1),
                                ais->type6.dac1fid18.portname),
                                json_stringify(buf2, sizeof (buf2),
                                ais->type6.dac1fid18.destination));
                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"lon\":%.3f,\"lat\":%.3f}\r\n",
                                ais->type6.dac1fid18.lon / AIS_LATLON3_DIV,
                                ais->type6.dac1fid18.lat / AIS_LATLON3_DIV);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"lon\":%d,\"lat\":%d}\r\n",
                                ais->type6.dac1fid18.lon,
                                ais->type6.dac1fid18.lat);
                        imo = true;
                        break;
                    case 20: /* IMO289 - Berthing Data */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"linkage\":%u,\"berth_length\":%u,"
                                "\"position\":%u,\"arrival\":\"%u-%uT%u:%u\","
                                "\"availability\":%u,"
                                "\"agent\":%u,\"fuel\":%u,\"chandler\":%u,"
                                "\"stevedore\":%u,\"electrical\":%u,"
                                "\"water\":%u,\"customs\":%u,\"cartage\":%u,"
                                "\"crane\":%u,\"lift\":%u,\"medical\":%u,"
                                "\"navrepair\":%u,\"provisions\":%u,"
                                "\"shiprepair\":%u,\"surveyor\":%u,"
                                "\"steam\":%u,\"tugs\":%u,\"solidwaste\":%u,"
                                "\"liquidwaste\":%u,\"hazardouswaste\":%u,"
                                "\"ballast\":%u,\"additional\":%u,"
                                "\"regional1\":%u,\"regional2\":%u,"
                                "\"future1\":%u,\"future2\":%u,"
                                "\"berth_name\":\"%s\",",
                                ais->type6.dac1fid20.linkage,
                                ais->type6.dac1fid20.berth_length,
                                ais->type6.dac1fid20.position,
                                ais->type6.dac1fid20.month,
                                ais->type6.dac1fid20.day,
                                ais->type6.dac1fid20.hour,
                                ais->type6.dac1fid20.minute,
                                ais->type6.dac1fid20.availability,
                                ais->type6.dac1fid20.agent,
                                ais->type6.dac1fid20.fuel,
                                ais->type6.dac1fid20.chandler,
                                ais->type6.dac1fid20.stevedore,
                                ais->type6.dac1fid20.electrical,
                                ais->type6.dac1fid20.water,
                                ais->type6.dac1fid20.customs,
                                ais->type6.dac1fid20.cartage,
                                ais->type6.dac1fid20.crane,
                                ais->type6.dac1fid20.lift,
                                ais->type6.dac1fid20.medical,
                                ais->type6.dac1fid20.navrepair,
                                ais->type6.dac1fid20.provisions,
                                ais->type6.dac1fid20.shiprepair,
                                ais->type6.dac1fid20.surveyor,
                                ais->type6.dac1fid20.steam,
                                ais->type6.dac1fid20.tugs,
                                ais->type6.dac1fid20.solidwaste,
                                ais->type6.dac1fid20.liquidwaste,
                                ais->type6.dac1fid20.hazardouswaste,
                                ais->type6.dac1fid20.ballast,
                                ais->type6.dac1fid20.additional,
                                ais->type6.dac1fid20.regional1,
                                ais->type6.dac1fid20.regional2,
                                ais->type6.dac1fid20.future1,
                                ais->type6.dac1fid20.future2,
                                json_stringify(buf1, sizeof (buf1),
                                ais->type6.dac1fid20.berth_name));
                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"berth_lon\":%.3f,"
                                "\"berth_lat\":%.3f,"
                                "\"berth_depth\":%.1f}\r\n",
                                ais->type6.dac1fid20.berth_lon / AIS_LATLON3_DIV,
                                ais->type6.dac1fid20.berth_lat / AIS_LATLON3_DIV,
                                ais->type6.dac1fid20.berth_depth * 0.1);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"berth_lon\":%d,"
                                "\"berth_lat\":%d,"
                                "\"berth_depth\":%u}\r\n",
                                ais->type6.dac1fid20.berth_lon,
                                ais->type6.dac1fid20.berth_lat,
                                ais->type6.dac1fid20.berth_depth);
                        imo = true;
                        break;
                    case 23: /* IMO289 - Area notice - addressed */
                        break;
                    case 25: /* IMO289 - Dangerous cargo indication */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"unit\":%u,\"amount\":%u,\"cargos\":[",
                                ais->type6.dac1fid25.unit,
                                ais->type6.dac1fid25.amount);
                        for (i = 0; i < (int) ais->type6.dac1fid25.ncargos; i++)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "{\"code\":%u,\"subtype\":%u},",

                                ais->type6.dac1fid25.cargos[i].code,
                                ais->type6.dac1fid25.cargos[i].subtype);
                        if (buf[strlen(buf) - 1] == ',')
                            buf[strlen(buf) - 1] = '\0';
                        (void) strlcat(buf, "]}\r\n", buflen);
                        imo = true;
                        break;
                    case 28: /* IMO289 - Route info - addressed */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"linkage\":%u,\"sender\":%u,",
                                ais->type6.dac1fid28.linkage,
                                ais->type6.dac1fid28.sender);
                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"rtype\":\"%s\",",
                                route_type[ais->type6.dac1fid28.rtype]);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"rtype\":%u,",
                                ais->type6.dac1fid28.rtype);
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"start\":\"%02u-%02uT%02u:%02uZ\",\"duration\":%u,\"waypoints\":[",
                                ais->type6.dac1fid28.month,
                                ais->type6.dac1fid28.day,
                                ais->type6.dac1fid28.hour,
                                ais->type6.dac1fid28.minute,
                                ais->type6.dac1fid28.duration);
                        for (i = 0; i < ais->type6.dac1fid28.waycount; i++) {
                            if (scaled_)
                                (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "{\"lon\":%.4f,\"lat\":%.4f},",
                                    ais->type6.dac1fid28.waypoints[i].lon / AIS_LATLON4_DIV,
                                    ais->type6.dac1fid28.waypoints[i].lat / AIS_LATLON4_DIV);
                            else
                                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "{\"lon\":%d,\"lat\":%d},",
                                    ais->type6.dac1fid28.waypoints[i].lon,
                                    ais->type6.dac1fid28.waypoints[i].lat);
                        }
                        if (buf[strlen(buf) - 1] == ',')
                            buf[strlen(buf) - 1] = '\0';
                        (void) strlcat(buf, "]}\r\n", buflen);
                        imo = true;
                        break;
                    case 30: /* IMO289 - Text description - addressed */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"linkage\":%u,\"text\":\"%s\"}\r\n",
                                ais->type6.dac1fid30.linkage,
                                json_stringify(buf1, sizeof (buf1),
                                ais->type6.dac1fid30.text));
                        imo = true;
                        break;
                    case 14: /* IMO236 - Tidal Window */
                    case 32: /* IMO289 - Tidal Window */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"month\":%u,\"day\":%u,\"tidals\":[",
                                ais->type6.dac1fid32.month,
                                ais->type6.dac1fid32.day);
                        for (i = 0; i < ais->type6.dac1fid32.ntidals; i++) {
                            const struct tidal_t *tp = &ais->type6.dac1fid32.tidals[i];
                            if (scaled_)
                                (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "{\"lon\":%.3f,\"lat\":%.3f,",
                                    tp->lon / AIS_LATLON3_DIV,
                                    tp->lat / AIS_LATLON3_DIV);
                            else
                                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "{\"lon\":%d,\"lat\":%d,",
                                    tp->lon,
                                    tp->lat);
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "\"from_hour\":%u,\"from_min\":%u,\"to_hour\":%u,\"to_min\":%u,\"cdir\":%u,",
                                    tp->from_hour,
                                    tp->from_min,
                                    tp->to_hour,
                                    tp->to_min,
                                    tp->cdir);
                            if (scaled_)
                                (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "\"cspeed\":%.1f},",
                                    tp->cspeed / 10.0);
                            else
                                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "\"cspeed\":%u},",
                                    tp->cspeed);
                        }
                        if (buf[strlen(buf) - 1] == ',')
                            buf[strlen(buf) - 1] = '\0';
                        (void) strlcat(buf, "]}\r\n", buflen);
                        imo = true;
                        break;
                }
            if (!imo)
                (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                    "\"data\":\"%zd:%s\"}\r\n",
                    ais->type6.bitcount,
                    json_stringify(buf1, sizeof (buf1),
                    gpsd_hexdump((char *) ais->type6.bitdata,
                    (ais->type6.bitcount + 7) / 8)));
        }
            break;
#endif /* SAD_ENABLE_MESSAGE_6  */
#ifdef SAD_ENABLE_MESSAGE_8       
        case 8: /* Binary Broadcast Message */
        {
          char buf1[JSON_VAL_MAX * 2 + 1];
          char buf2[JSON_VAL_MAX * 2 + 1];
          char buf3[JSON_VAL_MAX * 2 + 1];
         int  imo = false;
         int i;
            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                    "\"dac\":%u,\"fid\":%u,", ais->type8.dac, ais->type8.fid);
            if (ais->type8.dac == 1) {
                const char *trends[] = {
                    "steady",
                    "increasing",
                    "decreasing",
                    "N/A",
                };
                // WMO 306, Code table 4.201
                const char *preciptypes[] = {
                    "reserved",
                    "rain",
                    "thunderstorm",
                    "freezing rain",
                    "mixed/ice",
                    "snow",
                    "reserved",
                    "N/A",
                };
                const char *ice[] = {
                    "no",
                    "yes",
                    "reserved",
                    "N/A",
                };
                switch (ais->type8.fid) {
                    case 11: /* IMO236 - Meteorological/Hydrological data */
                        /* some fields have been merged to an ISO8601 partial date */
                        /* layout is almost identical to FID=31 from IMO289 */
                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"lat\":%.3f,\"lon\":%.3f,",
                                ais->type8.dac1fid11.lat / AIS_LATLON3_DIV,
                                ais->type8.dac1fid11.lon / AIS_LATLON3_DIV);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"lat\":%d,\"lon\":%d,",
                                ais->type8.dac1fid11.lat,
                                ais->type8.dac1fid11.lon);
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"timestamp\":\"%02uT%02u:%02uZ\","
                                "\"wspeed\":%u,\"wgust\":%u,\"wdir\":%u,"
                                "\"wgustdir\":%u,\"humidity\":%u,",
                                ais->type8.dac1fid11.day,
                                ais->type8.dac1fid11.hour,
                                ais->type8.dac1fid11.minute,
                                ais->type8.dac1fid11.wspeed,
                                ais->type8.dac1fid11.wgust,
                                ais->type8.dac1fid11.wdir,
                                ais->type8.dac1fid11.wgustdir,
                                ais->type8.dac1fid11.humidity);
                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"airtemp\":%.1f,\"dewpoint\":%.1f,"
                                "\"pressure\":%u,\"pressuretend\":\"%s\",",
                                (ais->type8.dac1fid11.airtemp - DAC1FID11_AIRTEMP_OFFSET) / DAC1FID11_AIRTEMP_DIV,
                                (ais->type8.dac1fid11.dewpoint - DAC1FID11_DEWPOINT_OFFSET) / DAC1FID11_DEWPOINT_DIV,
                                ais->type8.dac1fid11.pressure - DAC1FID11_PRESSURE_OFFSET,
                                trends[ais->type8.dac1fid11.pressuretend]);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"airtemp\":%u,\"dewpoint\":%u,"
                                "\"pressure\":%u,\"pressuretend\":%u,",
                                ais->type8.dac1fid11.airtemp,
                                ais->type8.dac1fid11.dewpoint,
                                ais->type8.dac1fid11.pressure,
                                ais->type8.dac1fid11.pressuretend);

                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"visibility\":%.1f,",
                                ais->type8.dac1fid11.visibility / DAC1FID11_VISIBILITY_DIV);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"visibility\":%u,",
                                ais->type8.dac1fid11.visibility);
                        if (!scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"waterlevel\":%d,",
                                ais->type8.dac1fid11.waterlevel);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"waterlevel\":%.1f,",
                                (ais->type8.dac1fid11.waterlevel - DAC1FID11_WATERLEVEL_OFFSET) / DAC1FID11_WATERLEVEL_DIV);

                        if (scaled_) {
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "\"leveltrend\":\"%s\","
                                    "\"cspeed\":%.1f,\"cdir\":%u,"
                                    "\"cspeed2\":%.1f,\"cdir2\":%u,\"cdepth2\":%u,"
                                    "\"cspeed3\":%.1f,\"cdir3\":%u,\"cdepth3\":%u,"
                                    "\"waveheight\":%.1f,\"waveperiod\":%u,\"wavedir\":%u,"
                                    "\"swellheight\":%.1f,\"swellperiod\":%u,\"swelldir\":%u,"
                                    "\"seastate\":%u,\"watertemp\":%.1f,"
                                    "\"preciptype\":\"%s\",\"salinity\":%.1f,\"ice\":\"%s\"",
                                    trends[ais->type8.dac1fid11.leveltrend],
                                    ais->type8.dac1fid11.cspeed / DAC1FID11_CSPEED_DIV,
                                    ais->type8.dac1fid11.cdir,
                                    ais->type8.dac1fid11.cspeed2 / DAC1FID11_CSPEED_DIV,
                                    ais->type8.dac1fid11.cdir2,
                                    ais->type8.dac1fid11.cdepth2,
                                    ais->type8.dac1fid11.cspeed3 / DAC1FID11_CSPEED_DIV,
                                    ais->type8.dac1fid11.cdir3,
                                    ais->type8.dac1fid11.cdepth3,
                                    ais->type8.dac1fid11.waveheight / DAC1FID11_WAVEHEIGHT_DIV,
                                    ais->type8.dac1fid11.waveperiod,
                                    ais->type8.dac1fid11.wavedir,
                                    ais->type8.dac1fid11.swellheight / DAC1FID11_WAVEHEIGHT_DIV,
                                    ais->type8.dac1fid11.swellperiod,
                                    ais->type8.dac1fid11.swelldir,
                                    ais->type8.dac1fid11.seastate,
                                    (ais->type8.dac1fid11.watertemp - DAC1FID11_WATERTEMP_OFFSET) / DAC1FID11_WATERTEMP_DIV,
                                    preciptypes[ais->type8.dac1fid11.preciptype],
                                    ais->type8.dac1fid11.salinity / DAC1FID11_SALINITY_DIV,
                                    ice[ais->type8.dac1fid11.ice]);
                        } else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"leveltrend\":%u,"
                                "\"cspeed\":%u,\"cdir\":%u,"
                                "\"cspeed2\":%u,\"cdir2\":%u,\"cdepth2\":%u,"
                                "\"cspeed3\":%u,\"cdir3\":%u,\"cdepth3\":%u,"
                                "\"waveheight\":%u,\"waveperiod\":%u,\"wavedir\":%u,"
                                "\"swellheight\":%u,\"swellperiod\":%u,\"swelldir\":%u,"
                                "\"seastate\":%u,\"watertemp\":%u,"
                                "\"preciptype\":%u,\"salinity\":%u,\"ice\":%u",
                                ais->type8.dac1fid11.leveltrend,
                                ais->type8.dac1fid11.cspeed,
                                ais->type8.dac1fid11.cdir,
                                ais->type8.dac1fid11.cspeed2,
                                ais->type8.dac1fid11.cdir2,
                                ais->type8.dac1fid11.cdepth2,
                                ais->type8.dac1fid11.cspeed3,
                                ais->type8.dac1fid11.cdir3,
                                ais->type8.dac1fid11.cdepth3,
                                ais->type8.dac1fid11.waveheight,
                                ais->type8.dac1fid11.waveperiod,
                                ais->type8.dac1fid11.wavedir,
                                ais->type8.dac1fid11.swellheight,
                                ais->type8.dac1fid11.swellperiod,
                                ais->type8.dac1fid11.swelldir,
                                ais->type8.dac1fid11.seastate,
                                ais->type8.dac1fid11.watertemp,
                                ais->type8.dac1fid11.preciptype,
                                ais->type8.dac1fid11.salinity,
                                ais->type8.dac1fid11.ice);
                        (void) strlcat(buf, "}\r\n", buflen);
                        imo = true;
                        break;
                    case 13: /* IMO236 - Fairway closed */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"reason\":\"%s\",\"closefrom\":\"%s\","
                                "\"closeto\":\"%s\",\"radius\":%u,"
                                "\"extunit\":%u,"
                                "\"from\":\"%02u-%02uT%02u:%02u\","
                                "\"to\":\"%02u-%02uT%02u:%02u\"}\r\n",
                                json_stringify(buf1, sizeof (buf1),
                                ais->type8.dac1fid13.reason),
                                json_stringify(buf2, sizeof (buf2),
                                ais->type8.dac1fid13.closefrom),
                                json_stringify(buf3, sizeof (buf3),
                                ais->type8.dac1fid13.closeto),
                                ais->type8.dac1fid13.radius,
                                ais->type8.dac1fid13.extunit,
                                ais->type8.dac1fid13.fmonth,
                                ais->type8.dac1fid13.fday,
                                ais->type8.dac1fid13.fhour,
                                ais->type8.dac1fid13.fminute,
                                ais->type8.dac1fid13.tmonth,
                                ais->type8.dac1fid13.tday,
                                ais->type8.dac1fid13.thour,
                                ais->type8.dac1fid13.tminute);
                        imo = true;
                        break;
                    case 15: /* IMO236 - Extended ship and voyage */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"airdraught\":%u}\r\n",
                                ais->type8.dac1fid15.airdraught);
                        imo = true;
                        break;
                    case 17: /* IMO289 - VTS-generated/synthetic targets */
                        (void) strlcat(buf, "\"targets\":[", buflen);
                        for (i = 0; i < ais->type8.dac1fid17.ntargets; i++) {
                            if (scaled_)
                                (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "{\"idtype\":\"%s\",",
                                    idtypes[ais->type8.dac1fid17.targets[i].idtype]);
                            else
                                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "{\"idtype\":%u,",
                                    ais->type8.dac1fid17.targets[i].idtype);
                            switch (ais->type8.dac1fid17.targets[i].idtype) {
                                case DAC1FID17_IDTYPE_MMSI:
                                    (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                            "\"%s\":\"%u\",",
                                            idtypes[ais->type8.dac1fid17.targets[i].idtype],
                                            ais->type8.dac1fid17.targets[i].id.mmsi);
                                    break;
                                case DAC1FID17_IDTYPE_IMO:
                                    (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                            "\"%s\":\"%u\",",
                                            idtypes[ais->type8.dac1fid17.targets[i].idtype],
                                            ais->type8.dac1fid17.targets[i].id.imo);
                                    break;
                                case DAC1FID17_IDTYPE_CALLSIGN:
                                    (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                            "\"%s\":\"%s\",",
                                            idtypes[ais->type8.dac1fid17.targets[i].idtype],
                                            json_stringify(buf1, sizeof (buf1),
                                            ais->type8.dac1fid17.targets[i].id.callsign));
                                    break;
                                default:
                                    (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                            "\"%s\":\"%s\",",
                                            idtypes[ais->type8.dac1fid17.targets[i].idtype],
                                            json_stringify(buf1, sizeof (buf1),
                                            ais->type8.dac1fid17.targets[i].id.other));
                            }
                            if (scaled_)
                                (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "\"lat\":%.3f,\"lon\":%.3f,",
                                    ais->type8.dac1fid17.targets[i].lat / AIS_LATLON3_DIV,
                                    ais->type8.dac1fid17.targets[i].lon / AIS_LATLON3_DIV);
                            else
                                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "\"lat\":%d,\"lon\":%d,",
                                    ais->type8.dac1fid17.targets[i].lat,
                                    ais->type8.dac1fid17.targets[i].lon);
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "\"course\":%u,\"second\":%u,\"speed\":%u},",
                                    ais->type8.dac1fid17.targets[i].course,
                                    ais->type8.dac1fid17.targets[i].second,
                                    ais->type8.dac1fid17.targets[i].speed);
                        }
                        if (buf[strlen(buf) - 1] == ',')
                            buf[strlen(buf) - 1] = '\0';
                        (void) strlcat(buf, "]}\r\n", buflen);
                        imo = true;
                        break;
                    case 19: /* IMO289 - Marine Traffic Signal */
                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"linkage\":%u,\"station\":\"%s\",\"lon\":%.3f,\"lat\":%.3f,\"status\":%u,\"signal\":\"%s\",\"hour\":%u,\"minute\":%u,\"nextsignal\":\"%s\"}\r\n",
                                ais->type8.dac1fid19.linkage,
                                json_stringify(buf1, sizeof (buf1),
                                ais->type8.dac1fid19.station),
                                ais->type8.dac1fid19.lon / AIS_LATLON3_DIV,
                                ais->type8.dac1fid19.lat / AIS_LATLON3_DIV,
                                ais->type8.dac1fid19.status,
                                SIGNAL_DISPLAY(ais->type8.dac1fid19.signal),
                                ais->type8.dac1fid19.hour,
                                ais->type8.dac1fid19.minute,
                                SIGNAL_DISPLAY(ais->type8.dac1fid19.nextsignal));
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"linkage\":%u,\"station\":\"%s\",\"lon\":%d,\"lat\":%d,\"status\":%u,\"signal\":%u,\"hour\":%u,\"minute\":%u,\"nextsignal\":%u}\r\n",
                                ais->type8.dac1fid19.linkage,
                                json_stringify(buf1, sizeof (buf1),
                                ais->type8.dac1fid19.station),
                                ais->type8.dac1fid19.lon,
                                ais->type8.dac1fid19.lat,
                                ais->type8.dac1fid19.status,
                                ais->type8.dac1fid19.signal,
                                ais->type8.dac1fid19.hour,
                                ais->type8.dac1fid19.minute,
                                ais->type8.dac1fid19.nextsignal);
                        imo = true;
                        break;
                    case 21: /* IMO289 - Weather obs. report from ship */
                        break;
                    case 22: /* IMO289 - Area notice - broadcast */
                        break;
                    case 24: /* IMO289 - Extended ship static & voyage-related data */
                        break;
                    case 25: /* IMO289 - Dangerous Cargo Indication */
                        break;
                    case 27: /* IMO289 - Route information - broadcast */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"linkage\":%u,\"sender\":%u,",
                                ais->type8.dac1fid27.linkage,
                                ais->type8.dac1fid27.sender);
                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"rtype\":\"%s\",",
                                route_type[ais->type8.dac1fid27.rtype]);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"rtype\":%u,",
                                ais->type8.dac1fid27.rtype);
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"start\":\"%02u-%02uT%02u:%02uZ\",\"duration\":%u,\"waypoints\":[",
                                ais->type8.dac1fid27.month,
                                ais->type8.dac1fid27.day,
                                ais->type8.dac1fid27.hour,
                                ais->type8.dac1fid27.minute,
                                ais->type8.dac1fid27.duration);
                        for (i = 0; i < ais->type8.dac1fid27.waycount; i++) {
                            if (scaled_)
                                (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "{\"lon\":%.4f,\"lat\":%.4f},",
                                    ais->type8.dac1fid27.waypoints[i].lon / AIS_LATLON4_DIV,
                                    ais->type8.dac1fid27.waypoints[i].lat / AIS_LATLON4_DIV);
                            else
                                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "{\"lon\":%d,\"lat\":%d},",
                                    ais->type8.dac1fid27.waypoints[i].lon,
                                    ais->type8.dac1fid27.waypoints[i].lat);
                        }
                        if (buf[strlen(buf) - 1] == ',')
                            buf[strlen(buf) - 1] = '\0';
                        (void) strlcat(buf, "]}\r\n", buflen);
                        imo = true;
                        break;
                    case 29: /* IMO289 - Text Description - broadcast */
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"linkage\":%u,\"text\":\"%s\"}\r\n",
                                ais->type8.dac1fid29.linkage,
                                json_stringify(buf1, sizeof (buf1),
                                ais->type8.dac1fid29.text));
                        imo = true;
                        break;
                    case 31: /* IMO289 - Meteorological/Hydrological data */
                        /* some fields have been merged to an ISO8601 partial date */
                        /* layout is almost identical to FID=11 from IMO236 */
                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"lat\":%.3f,\"lon\":%.3f,",
                                ais->type8.dac1fid31.lat / AIS_LATLON3_DIV,
                                ais->type8.dac1fid31.lon / AIS_LATLON3_DIV);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"lat\":%d,\"lon\":%d,",
                                ais->type8.dac1fid31.lat,
                                ais->type8.dac1fid31.lon);
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"accuracy\":%s,",
                                JSON_BOOL(ais->type8.dac1fid31.accuracy));
                        (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"timestamp\":\"%02uT%02u:%02uZ\","
                                "\"wspeed\":%u,\"wgust\":%u,\"wdir\":%u,"
                                "\"wgustdir\":%u,\"humidity\":%u,",
                                ais->type8.dac1fid31.day,
                                ais->type8.dac1fid31.hour,
                                ais->type8.dac1fid31.minute,
                                ais->type8.dac1fid31.wspeed,
                                ais->type8.dac1fid31.wgust,
                                ais->type8.dac1fid31.wdir,
                                ais->type8.dac1fid31.wgustdir,
                                ais->type8.dac1fid31.humidity);
                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"airtemp\":%.1f,\"dewpoint\":%.1f,"
                                "\"pressure\":%u,\"pressuretend\":\"%s\","
                                "\"visgreater\":%s,",
                                ais->type8.dac1fid31.airtemp / DAC1FID31_AIRTEMP_DIV,
                                ais->type8.dac1fid31.dewpoint / DAC1FID31_DEWPOINT_DIV,
                                ais->type8.dac1fid31.pressure - DAC1FID31_PRESSURE_OFFSET,
                                trends[ais->type8.dac1fid31.pressuretend],
                                JSON_BOOL(ais->type8.dac1fid31.visgreater));
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"airtemp\":%d,\"dewpoint\":%d,"
                                "\"pressure\":%u,\"pressuretend\":%u,"
                                "\"visgreater\":%s,",
                                ais->type8.dac1fid31.airtemp,
                                ais->type8.dac1fid31.dewpoint,
                                ais->type8.dac1fid31.pressure,
                                ais->type8.dac1fid31.pressuretend,
                                JSON_BOOL(ais->type8.dac1fid31.visgreater));

                        if (scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"visibility\":%.1f,",
                                ais->type8.dac1fid31.visibility / DAC1FID31_VISIBILITY_DIV);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"visibility\":%u,",
                                ais->type8.dac1fid31.visibility);
                        if (!scaled_)
                            (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"waterlevel\":%d,",
                                ais->type8.dac1fid31.waterlevel);
                        else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"waterlevel\":%.1f,",
                                (ais->type8.dac1fid31.waterlevel - DAC1FID31_WATERLEVEL_OFFSET) / DAC1FID31_WATERLEVEL_DIV);

                        if (scaled_) {
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                    "\"leveltrend\":\"%s\","
                                    "\"cspeed\":%.1f,\"cdir\":%u,"
                                    "\"cspeed2\":%.1f,\"cdir2\":%u,\"cdepth2\":%u,"
                                    "\"cspeed3\":%.1f,\"cdir3\":%u,\"cdepth3\":%u,"
                                    "\"waveheight\":%.1f,\"waveperiod\":%u,\"wavedir\":%u,"
                                    "\"swellheight\":%.1f,\"swellperiod\":%u,\"swelldir\":%u,"
                                    "\"seastate\":%u,\"watertemp\":%.1f,"
                                    "\"preciptype\":\"%s\",\"salinity\":%.1f,\"ice\":\"%s\"",
                                    trends[ais->type8.dac1fid31.leveltrend],
                                    ais->type8.dac1fid31.cspeed / DAC1FID31_CSPEED_DIV,
                                    ais->type8.dac1fid31.cdir,
                                    ais->type8.dac1fid31.cspeed2 / DAC1FID31_CSPEED_DIV,
                                    ais->type8.dac1fid31.cdir2,
                                    ais->type8.dac1fid31.cdepth2,
                                    ais->type8.dac1fid31.cspeed3 / DAC1FID31_CSPEED_DIV,
                                    ais->type8.dac1fid31.cdir3,
                                    ais->type8.dac1fid31.cdepth3,
                                    ais->type8.dac1fid31.waveheight / DAC1FID31_HEIGHT_DIV,
                                    ais->type8.dac1fid31.waveperiod,
                                    ais->type8.dac1fid31.wavedir,
                                    ais->type8.dac1fid31.swellheight / DAC1FID31_HEIGHT_DIV,
                                    ais->type8.dac1fid31.swellperiod,
                                    ais->type8.dac1fid31.swelldir,
                                    ais->type8.dac1fid31.seastate,
                                    ais->type8.dac1fid31.watertemp / DAC1FID31_WATERTEMP_DIV,
                                    preciptypes[ais->type8.dac1fid31.preciptype],
                                    ais->type8.dac1fid31.salinity / DAC1FID31_SALINITY_DIV,
                                    ice[ais->type8.dac1fid31.ice]);
                        } else
                            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                                "\"leveltrend\":%u,"
                                "\"cspeed\":%u,\"cdir\":%u,"
                                "\"cspeed2\":%u,\"cdir2\":%u,\"cdepth2\":%u,"
                                "\"cspeed3\":%u,\"cdir3\":%u,\"cdepth3\":%u,"
                                "\"waveheight\":%u,\"waveperiod\":%u,\"wavedir\":%u,"
                                "\"swellheight\":%u,\"swellperiod\":%u,\"swelldir\":%u,"
                                "\"seastate\":%u,\"watertemp\":%d,"
                                "\"preciptype\":%u,\"salinity\":%u,\"ice\":%u",
                                ais->type8.dac1fid31.leveltrend,
                                ais->type8.dac1fid31.cspeed,
                                ais->type8.dac1fid31.cdir,
                                ais->type8.dac1fid31.cspeed2,
                                ais->type8.dac1fid31.cdir2,
                                ais->type8.dac1fid31.cdepth2,
                                ais->type8.dac1fid31.cspeed3,
                                ais->type8.dac1fid31.cdir3,
                                ais->type8.dac1fid31.cdepth3,
                                ais->type8.dac1fid31.waveheight,
                                ais->type8.dac1fid31.waveperiod,
                                ais->type8.dac1fid31.wavedir,
                                ais->type8.dac1fid31.swellheight,
                                ais->type8.dac1fid31.swellperiod,
                                ais->type8.dac1fid31.swelldir,
                                ais->type8.dac1fid31.seastate,
                                ais->type8.dac1fid31.watertemp,
                                ais->type8.dac1fid31.preciptype,
                                ais->type8.dac1fid31.salinity,
                                ais->type8.dac1fid31.ice);
                        (void) strlcat(buf, "}\r\n", buflen);
                        imo = true;
                        break;
                }
            }
            if (!imo)
                (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                    "\"data\":\"%zd:%s\"}\r\n",
                    ais->type8.bitcount,
                    json_stringify(buf1, sizeof (buf1),
                    gpsd_hexdump((char *) ais->type8.bitdata,
                    (ais->type8.bitcount + 7) / 8)));
        }
            break;
#endif /* SAD_ENABLE_MESSAGE_8  */  
            
#ifdef SAD_ENABLE_MESSAGE_9            
        case 9: /* Standard SAR Aircraft Position Report */
            if (scaled_) {
                char altlegend[20];
                char speedlegend[20];

                /*
                 * Express altitude as nan if not available,
                 * "high" for above the reporting ceiling.
                 */
                if (ais->type9.alt == AIS_ALT_NOT_AVAILABLE)
                    (void)strlcpy(altlegend, "\"nan\"", sizeof (altlegend));
                else if (ais->type9.alt == AIS_ALT_HIGH)
                    (void)strlcpy(altlegend, "\"high\"", sizeof (altlegend));
                else
                    (void) snprintf(altlegend, sizeof (altlegend),
                        "%u", ais->type9.alt);

                /*
                 * Express speed as nan if not available,
                 * "high" for above the reporting ceiling.
                 */
                if (ais->type9.speed == AIS_SAR_SPEED_NOT_AVAILABLE)
                    (void)strlcpy(speedlegend, "\"nan\"", sizeof (speedlegend));
                else if (ais->type9.speed == AIS_SAR_FAST_MOVER)
                    (void)strlcpy(speedlegend, "\"fast\"", sizeof (speedlegend));
                else
                    (void) snprintf(speedlegend, sizeof (speedlegend),
                        "%u", ais->type1.speed);

                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                        "\"alt\":%s,\"speed\":%s,\"accuracy\":%s,"
                        "\"lon\":%.4f,\"lat\":%.4f,\"course\":%.1f,"
                        "\"second\":%u,\"regional\":%u,\"dte\":%u,"
                        "\"raim\":%s,\"radio\":%u}\r\n",
                        altlegend,
                        speedlegend,
                        JSON_BOOL(ais->type9.accuracy),
                        ais->type9.lon / AIS_LATLON_DIV,
                        ais->type9.lat / AIS_LATLON_DIV,
                        ais->type9.course / 10.0,
                        ais->type9.second,
                        ais->type9.regional,
                        ais->type9.dte,
                        JSON_BOOL(ais->type9.raim), ais->type9.radio);
            } else {
                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                        "\"alt\":%u,\"speed\":%u,\"accuracy\":%s,"
                        "\"lon\":%d,\"lat\":%d,\"course\":%u,"
                        "\"second\":%u,\"regional\":%u,\"dte\":%u,"
                        "\"raim\":%s,\"radio\":%u}\r\n",
                        ais->type9.alt,
                        ais->type9.speed,
                        JSON_BOOL(ais->type9.accuracy),
                        ais->type9.lon,
                        ais->type9.lat,
                        ais->type9.course,
                        ais->type9.second,
                        ais->type9.regional,
                        ais->type9.dte,
                        JSON_BOOL(ais->type9.raim), ais->type9.radio);
            }
            break;
#endif /* SAD_ENABLE_MESSAGE_9 */
#ifdef SAD_ENABLE_MESSAGE_18            
        case 18:
            if (scaled_) {
                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                        "\"reserved\":%u,\"speed\":%.1f,\"accuracy\":%s,"
                        "\"lon\":%.4f,\"lat\":%.4f,\"course\":%.1f,"
                        "\"heading\":%u,\"second\":%u,\"regional\":%u,"
                        "\"cs\":%s,\"display\":%s,\"dsc\":%s,\"band\":%s,"
                        "\"msg22\":%s,\"raim\":%s,\"radio\":%u}\r\n",
                        ais->type18.reserved,
                        ais->type18.speed / 10.0,
                        JSON_BOOL(ais->type18.accuracy),
                        ais->type18.lon / AIS_LATLON_DIV,
                        ais->type18.lat / AIS_LATLON_DIV,
                        ais->type18.course / 10.0,
                        ais->type18.heading,
                        ais->type18.second,
                        ais->type18.regional,
                        JSON_BOOL(ais->type18.cs),
                        JSON_BOOL(ais->type18.display),
                        JSON_BOOL(ais->type18.dsc),
                        JSON_BOOL(ais->type18.band),
                        JSON_BOOL(ais->type18.msg22),
                        JSON_BOOL(ais->type18.raim), ais->type18.radio);
            } else {
                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                        "\"reserved\":%u,\"speed\":%u,\"accuracy\":%s,"
                        "\"lon\":%d,\"lat\":%d,\"course\":%u,"
                        "\"heading\":%u,\"second\":%u,\"regional\":%u,"
                        "\"cs\":%s,\"display\":%s,\"dsc\":%s,\"band\":%s,"
                        "\"msg22\":%s,\"raim\":%s,\"radio\":%u}\r\n",
                        ais->type18.reserved,
                        ais->type18.speed,
                        JSON_BOOL(ais->type18.accuracy),
                        ais->type18.lon,
                        ais->type18.lat,
                        ais->type18.course,
                        ais->type18.heading,
                        ais->type18.second,
                        ais->type18.regional,
                        JSON_BOOL(ais->type18.cs),
                        JSON_BOOL(ais->type18.display),
                        JSON_BOOL(ais->type18.dsc),
                        JSON_BOOL(ais->type18.band),
                        JSON_BOOL(ais->type18.msg22),
                        JSON_BOOL(ais->type18.raim), ais->type18.radio);
            }
            break;
#endif /* SAD_ENABLE_MESSAGE_18 */
#ifdef SAD_ENABLE_MESSAGE_19
        case 19:
        {
          char buf1[JSON_VAL_MAX * 2 + 1];
            if (scaled_) {
                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                        "\"reserved\":%u,\"speed\":%.1f,\"accuracy\":%s,"
                        "\"lon\":%.4f,\"lat\":%.4f,\"course\":%.1f,"
                        "\"heading\":%u,\"second\":%u,\"regional\":%u,"
                        "\"shipname\":\"%s\",\"shiptype\":\"%s\","
                        "\"to_bow\":%u,\"to_stern\":%u,\"to_port\":%u,"
                        "\"to_starboard\":%u,\"epfd\":\"%s\",\"raim\":%s,"
                        "\"dte\":%u,\"assigned\":%s}\r\n",
                        ais->type19.reserved,
                        ais->type19.speed / 10.0,
                        JSON_BOOL(ais->type19.accuracy),
                        ais->type19.lon / AIS_LATLON_DIV,
                        ais->type19.lat / AIS_LATLON_DIV,
                        ais->type19.course / 10.0,
                        ais->type19.heading,
                        ais->type19.second,
                        ais->type19.regional,
                        json_stringify(buf1, sizeof (buf1),
                        ais->type19.shipname),
                        SHIPTYPE_DISPLAY(ais->type19.shiptype),
                        ais->type19.to_bow,
                        ais->type19.to_stern,
                        ais->type19.to_port,
                        ais->type19.to_starboard,
                        EPFD_DISPLAY(ais->type19.epfd),
                        JSON_BOOL(ais->type19.raim),
                        ais->type19.dte, JSON_BOOL(ais->type19.assigned));
            } else {
                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                        "\"reserved\":%u,\"speed\":%u,\"accuracy\":%s,"
                        "\"lon\":%d,\"lat\":%d,\"course\":%u,"
                        "\"heading\":%u,\"second\":%u,\"regional\":%u,"
                        "\"shipname\":\"%s\",\"shiptype\":%u,"
                        "\"to_bow\":%u,\"to_stern\":%u,\"to_port\":%u,"
                        "\"to_starboard\":%u,\"epfd\":%u,\"raim\":%s,"
                        "\"dte\":%u,\"assigned\":%s}\r\n",
                        ais->type19.reserved,
                        ais->type19.speed,
                        JSON_BOOL(ais->type19.accuracy),
                        ais->type19.lon,
                        ais->type19.lat,
                        ais->type19.course,
                        ais->type19.heading,
                        ais->type19.second,
                        ais->type19.regional,
                        json_stringify(buf1, sizeof (buf1),
                        ais->type19.shipname),
                        ais->type19.shiptype,
                        ais->type19.to_bow,
                        ais->type19.to_stern,
                        ais->type19.to_port,
                        ais->type19.to_starboard,
                        ais->type19.epfd,
                        JSON_BOOL(ais->type19.raim),
                        ais->type19.dte, JSON_BOOL(ais->type19.assigned));
            }
        }
            break;
#endif /* SAD_ENABLE_MESSAGE_19 */
#ifdef SAD_ENABLE_MESSAGE_26
        case 26: /* Binary Message, Multiple Slot */
            (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                    "\"addressed\":%s,\"structured\":%s,\"dest_mmsi\":%u,"
                    "\"app_id\":%u,\"data\":\"%zd:%s\",\"radio\":%u}\r\n",
                    JSON_BOOL(ais->type26.addressed),
                    JSON_BOOL(ais->type26.structured),
                    ais->type26.dest_mmsi,
                    ais->type26.app_id,
                    ais->type26.bitcount,
                    gpsd_hexdump((char *) ais->type26.bitdata,
                    (ais->type26.bitcount + 7) / 8),
                    ais->type26.radio);
            break;
#endif /* SAD_ENABLE_MESSAGE_26 */
#ifdef SAD_ENABLE_MESSAGE_27            
        case 27: /* Long Range AIS Broadcast message */
            if (scaled_)
                (void)snprintf(buf + strlen(buf), buflen - strlen(buf),
                    "\"status\":\"%s\","
                    "\"accuracy\":%s,\"lon\":%.1f,\"lat\":%.1f,"
                    "\"speed\":%u,\"course\":%u,\"raim\":%s,\"gnss\":%s}\r\n",
                    nav_legends[ais->type27.status],
                    JSON_BOOL(ais->type27.accuracy),
                    ais->type27.lon / AIS_LONGRANGE_LATLON_DIV,
                    ais->type27.lat / AIS_LONGRANGE_LATLON_DIV,
                    ais->type27.speed,
                    ais->type27.course,
                    JSON_BOOL(ais->type27.raim),
                    JSON_BOOL(ais->type27.gnss));
            else
                (void) snprintf(buf + strlen(buf), buflen - strlen(buf),
                    "\"status\":%u,"
                    "\"accuracy\":%s,\"lon\":%d,\"lat\":%d,"
                    "\"speed\":%u,\"course\":%u,\"raim\":%s,\"gnss\":%s}\r\n",
                    ais->type27.status,
                    JSON_BOOL(ais->type27.accuracy),
                    ais->type27.lon,
                    ais->type27.lat,
                    ais->type27.speed,
                    ais->type27.course,
                    JSON_BOOL(ais->type27.raim),
                    JSON_BOOL(ais->type27.gnss));
            break;
#endif /* SAD_ENABLE_MESSAGE_27 */            
        default:
            if (buf[strlen(buf) - 1] == ',')
                buf[strlen(buf) - 1] = '\0';
            (void) strlcat(buf, "}\r\n", buflen);
            break;
    } /* end switch */
    
}



/* get 0-origin big-endian words relative to start of packet buffer */
#define getword(i) (short)(lexer->inbuffer[2*(i)] | (lexer->inbuffer[2*(i)+1] << 8))

/* entry points begin here */







#ifdef SAD_STRICT_DECODE
SADERR(TO_BE_IMPLEMENTED);
SADERR(MISSING_FIELD);
SADERR(MMSI_0);
SADERR(BAD_RADIO_CHANNEL);
SADERR(MESSAGE123_PAYLOAD_NOT168);
SADERR(MESSAGE5_PAYLOAD_NOT424);
SADERR(MESSAGE8_PAYLOAD_NOT56_1008);
SADERR(MESSAGE18_PAYLOAD_NOT168);
SADERR(MESSAGE6_PAYLOAD_NOT88_1008);
SADERR(MESSAGE9_PAYLOAD_NOT168);
#endif

int sad_filter_init(sad_filter_t* f_, int (*f_ais_cb_)(struct sad_filter_s*), 
                    void* userdata_, void (*f_error_cb)(const char*)) {
    if (NULL == f_ || NULL == f_ais_cb_) return 1;
    memset(f_, 0, sizeof (sad_filter_t));
    f_->f_ais_cb = f_ais_cb_;
    f_->f_error_cb = f_error_cb;
    f_->userdata = userdata_;
    return 0;
}

#define SADERRM(FMTERRM) \
  do {                                                                        \
    int iii;                                                                  \
    char bufss[1024];                                                         \
    iii = snprintf(bufss,sentence->n,"%s", sentence->start);                  \
    iii = asprintf(&mmerr,                                                    \
                   #FMTERRM "{%s:%d:%d}%s:%d\n",                              \
                   bufss,                                                     \
                   (int)sentence->n,                                          \
                   (int)sentence->start[0],                                   \
                   __FILE__,                                                  \
                   __LINE__ );                                                \
    (void)iii;                                                                \
    goto endline;                                                             \
  }                                                                           \
  while (0)

    
    
int sad_decode_multiline(sad_filter_t* filter_, const char* buffer_, size_t n_) {

    sub0_line_t sentences;
    sub0_substring_t* sentence = NULL;
    sub0_line_prepare(buffer_, n_, '\n', &sentences);

    while (NULL != (sentence = sub0_line_next_substring(&sentences))) {
        char* mmerr = NULL;
        
        /* level 1: incoherent size ... garbage */
        if (NMEA_MIN > sentence->n || NMEA_MAX < sentence->n) continue;
        ++filter_->sentences;

        /* level 2: strip pre-garbage char */
        size_t s2_n = sentence->n;
        const char* s2_p = sentence->start;

        /* strip unwanted pre-characters */
        while (('!' != *s2_p) && (s2_p < sentence->end)) {
            ++s2_p;
            --s2_n;
        }

        /* we should have found ! here*/
        if (s2_p == sentence->end || s2_n < 16) {
            
            SADERRM(NO_AIVDM);
        }
        ++s2_p;
        --s2_n;

        /* checksum + number field count */
        {
            int nb_comas = 0;

            size_t count_check_sum = s2_n;
            int crc = 0;
            char csum[3] = {'0', '0', '0'};
            const char* c = s2_p;
            while (*c != '*') {

                if (',' == *c) ++nb_comas;

                crc ^= *c;
                --count_check_sum;
                if (2 > count_check_sum) {
                    SADERRM(BAD_CRC);
                }
                ++c;
            }
#ifdef SAD_STRICT_DECODE                    
            if (6 != nb_comas) {
                SADERRM(MISSING_FIELD);
            }
#endif            
            (void) snprintf(csum, sizeof (csum), "%02X", crc);
            ++c;
            if (csum[0] != toupper(*c)) {
                SADERRM(BAD_CRC);
            };
            ++c;
            if (csum[1] != toupper(*c)) {
                SADERRM(BAD_CRC);
            };
        }
        /* FIELD 1 check AIVDM, */
        if ('A' != *s2_p) {
            SADERRM(NO_AIVDM);
        }
        ++s2_p;
        --s2_n;
        if ('I' != *s2_p) {
            SADERRM(NO_AIVDM);
        }
        ++s2_p;
        --s2_n;
        if ('V' != *s2_p) {
            SADERRM(NO_AIVDM);
        }
        ++s2_p;
        --s2_n;
        if ('D' != *s2_p) {
            SADERRM(NO_AIVDM);
        }
        ++s2_p;
        --s2_n;
        if ('M' != *s2_p) {
            SADERRM(NO_AIVDM);
        }
        ++s2_p;
        --s2_n;
        if (',' != *s2_p) {
            SADERRM(NO_AIVDM);
        }
        ++s2_p;
        --s2_n;

        sub0_line_t fields;
        sub0_substring_t* field = NULL;
        sub0_line_prepare(s2_p, s2_n, ',', &fields);

        /* FIELD 2: fragment count */
        field = sub0_line_next_substring(&fields);
        filter_->frag_cur.seq_len = *(field->start) - '0';
        
        
        /* FIELD 3: fragment number */
        field = sub0_line_next_substring(&fields);
        filter_->frag_cur.seq_cur = *(field->start) - '0';

        /* FIELD 4: seq message id */
        field = sub0_line_next_substring(&fields);
        filter_->frag_cur.seq_id = (0 == field->n) ? 0 : *(field->start) - '0';

        /* FIELD 5: radio channel code */
        field = sub0_line_next_substring(&fields);
        filter_->frag_cur.seq_chan = (0 == field->n) ? '0' : field->start[0];
        
        
        /* do not decode frags */
        if (1 != filter_->frag_cur.seq_len) {
            ++filter_->frags;
            goto endline;
        }
        

        /* FIELD 6: payload */
        field = sub0_line_next_substring(&fields);
        const unsigned char *data = (const unsigned char*) field->start;
        const size_t data_n = field->n;

        /* FIELD 7: x*CRC */
        field = sub0_line_next_substring(&fields);
        unsigned char pad = field->start[0];

        /* payload  decode */
        {
            /* payload sixbit decode */
            const unsigned char *cp;
            unsigned char ch;
            struct aivdm_context_t ais_context = {0};
            ais_context.bitlen = 0;
            int i;

            /* wacky 6-bit encoding, shades of FIELDATA */
            for (cp = data; cp < data + data_n; cp++) {
                ch = *cp;
                ch -= 48;
                if (ch >= 40)
                    ch -= 8;

                for (i = 5; i >= 0; i--) {
                    if ((ch >> i) & 0x01) {
                        ais_context.bits[ais_context.bitlen / 8] |=
                                (1 << (7 - ais_context.bitlen % 8)); /* to improve */
                    }
                    ais_context.bitlen++;
                    if (ais_context.bitlen > sizeof (ais_context.bits)) {
                        SADERRM(OVERLONG_AIVDM_PAYLOAD);
                    }
                }
            }
            if (isdigit(pad))
                ais_context.bitlen -= (pad - '0'); /* ASCII assumption */

            /* payload binary decode */
            {
                //                unsigned int u;
                memset(&filter_->ais, 0, sizeof (struct ais_t));
                struct ais_t *ais = &filter_->ais;
#define MMUBITS(s, l)	ubits((unsigned char *)ais_context.bits, s, l)
#define MMSBITS(s, l)	sbits((signed char *)ais_context.bits, s, l)
#define MMUCHARS(s, to)	from_sixbit((unsigned char *)ais_context.bits, s, sizeof(to), to)                

                /* ais type */
                ais->type = MMUBITS(0, 6);

                if (0 == ais->type || ais->type > AIVDM_MESSAGES_TYPE) {
                    SADERRM(MESSAGE_TYPE_0);
                }
                /* ais type */
                ais->repeat = MMUBITS(6, 2);
                /* ais mmsi */
                ais->mmsi = MMUBITS(8, 30);

#ifdef SAD_STRICT_DECODE                     
                if (0 == ais->mmsi) {
                    mmerr = MMSI_0;
                    goto endline;
                }
#endif                

                

                switch (ais->type) {

                        /**
                         * message 1,2,3
                         */
                    case 1: /* Position Report */
                    case 2:
                    case 3:
#ifdef SAD_STRICT_DECODE            
                        if (ais_context.bitlen != 168) {
                            mmerr = MESSAGE123_PAYLOAD_NOT168;
                            printf("%zd ais_context.bitlen\n", ais_context.bitlen);
                            goto endline;
                        }
#endif            
                        ais->type1.status = MMUBITS(38, 4);
                        ais->type1.turn = MMSBITS(42, 8);
                        ais->type1.speed = MMUBITS(50, 10);
                        ais->type1.accuracy = MMUBITS(60, 1) != 0;
                        ais->type1.lon = MMSBITS(61, 28);
                        ais->type1.lat = MMSBITS(89, 27);
                        ais->type1.course = MMUBITS(116, 12);
                        ais->type1.heading = MMUBITS(128, 9);
                        ais->type1.second = MMUBITS(137, 6);
                        ais->type1.maneuver = MMUBITS(143, 2);
                        //ais->type1.spare	        = UBITS(145, 3);
                        ais->type1.raim = MMUBITS(148, 1) != 0;
                        ais->type1.radio = MMUBITS(149, 20);
                        break;


#ifdef SAD_ENABLE_MESSAGE_5                  
                        /**
                         * message 5
                         */
                    case 5: /* Ship static and voyage related data */
#ifdef SAD_STRICT_DECODE              
                        if (ais_context.bitlen != 424) {
                            SADERRM(MESSAGE5_PAYLOAD_NOT424);
                        }
#endif            
                        ais->type5.ais_version = MMUBITS(38, 2);
                        ais->type5.imo = MMUBITS(40, 30);
                        MMUCHARS(70, ais->type5.callsign);
                        MMUCHARS(112, ais->type5.shipname);
                        ais->type5.shiptype = MMUBITS(232, 8);
                        ais->type5.to_bow = MMUBITS(240, 9);
                        ais->type5.to_stern = MMUBITS(249, 9);
                        ais->type5.to_port = MMUBITS(258, 6);
                        ais->type5.to_starboard = MMUBITS(264, 6);
                        ais->type5.epfd = MMUBITS(270, 4);
                        ais->type5.month = MMUBITS(274, 4);
                        ais->type5.day = MMUBITS(278, 5);
                        ais->type5.hour = MMUBITS(283, 5);
                        ais->type5.minute = MMUBITS(288, 6);
                        ais->type5.draught = MMUBITS(294, 8);
                        MMUCHARS(302, ais->type5.destination);
                        ais->type5.dte = MMUBITS(422, 1);
                        //ais->type5.spare        = UBITS(423, 1);
                        break;
#endif /* SAD_ENABLE_MESSAGE_5 */                       


#ifdef SAD_ENABLE_MESSAGE_27                  
                        /**
                         * message 27
                         */
                    case 27: /* IMO289 - Route information - broadcast */
                        ais->type8.dac1fid27.linkage = MMUBITS(56, 10);
                        ais->type8.dac1fid27.sender = MMUBITS(66, 3);
                        ais->type8.dac1fid27.rtype = MMUBITS(69, 5);
                        ais->type8.dac1fid27.month = MMUBITS(74, 4);
                        ais->type8.dac1fid27.day = MMUBITS(78, 5);
                        ais->type8.dac1fid27.hour = MMUBITS(83, 5);
                        ais->type8.dac1fid27.minute = MMUBITS(88, 6);
                        ais->type8.dac1fid27.duration = MMUBITS(94, 18);
                        ais->type8.dac1fid27.waycount = MMUBITS(112, 5);
#define ARRAY_BASE 117
#define ELEMENT_SIZE 55
                        for (i = 0; i < ais->type8.dac1fid27.waycount; i++) {
                            int a = ARRAY_BASE + (ELEMENT_SIZE * i);
                            ais->type8.dac1fid27.waypoints[i].lon = MMSBITS(a + 0, 28);
                            ais->type8.dac1fid27.waypoints[i].lat = MMSBITS(a + 28, 27);
                        }
#undef ARRAY_BASE
#undef ELEMENT_SIZE
                        break;

#endif /* SAD_ENABLE_MESSAGE_27 */

#ifdef SAD_ENABLE_MESSAGE_18                        
                        /**
                         * message 18
                         */
                    case 18: /* Standard Class B CS Position Report */

#ifdef SAD_STRICT_DECODE              
                        if (ais_context.bitlen != 168) {
                            SADERRM(MESSAGE18_PAYLOAD_NOT168);
                        }
#endif             
                        ais->type18.reserved = MMUBITS(38, 8);
                        ais->type18.speed = MMUBITS(46, 10);
                        ais->type18.accuracy = MMUBITS(56, 1) != 0;
                        ais->type18.lon = MMSBITS(57, 28);
                        ais->type18.lat = MMSBITS(85, 27);
                        ais->type18.course = MMUBITS(112, 12);
                        ais->type18.heading = MMUBITS(124, 9);
                        ais->type18.second = MMUBITS(133, 6);
                        ais->type18.regional = MMUBITS(139, 2);
                        ais->type18.cs = MMUBITS(141, 1) != 0;
                        ais->type18.display = MMUBITS(142, 1) != 0;
                        ais->type18.dsc = MMUBITS(143, 1) != 0;
                        ais->type18.band = MMUBITS(144, 1) != 0;
                        ais->type18.msg22 = MMUBITS(145, 1) != 0;
                        ais->type18.assigned = MMUBITS(146, 1) != 0;
                        ais->type18.raim = MMUBITS(147, 1) != 0;
                        ais->type18.radio = MMUBITS(148, 20);
                        break;

#endif /* SAD_ENABLE_MESSAGE_18 */
                        
#ifdef SAD_ENABLE_MESSAGE_19                       
                        /**
                         * message 19
                         */
                    case 19: /* Extended Class B CS Position Report */
#ifdef SAD_STRICT_DECODE              
                        if (ais_context.bitlen != 312) {
                            SADERRM(MESSAGE19_PAYLOAD_NOT312);
                        }
#endif             
                        ais->type19.reserved = MMUBITS(38, 8);
                        ais->type19.speed = MMUBITS(46, 10);
                        ais->type19.accuracy = MMUBITS(56, 1) != 0;
                        ais->type19.lon = MMSBITS(57, 28);
                        ais->type19.lat = MMSBITS(85, 27);
                        ais->type19.course = MMUBITS(112, 12);
                        ais->type19.heading = MMUBITS(124, 9);
                        ais->type19.second = MMUBITS(133, 6);
                        ais->type19.regional = MMUBITS(139, 4);
                        MMUCHARS(143, ais->type19.shipname);
                        ais->type19.shiptype = MMUBITS(263, 8);
                        ais->type19.to_bow = MMUBITS(271, 9);
                        ais->type19.to_stern = MMUBITS(280, 9);
                        ais->type19.to_port = MMUBITS(289, 6);
                        ais->type19.to_starboard = MMUBITS(295, 6);
                        ais->type19.epfd = MMUBITS(299, 4);
                        ais->type19.raim = MMUBITS(302, 1) != 0;
                        ais->type19.dte = MMUBITS(305, 1) != 0;
                        ais->type19.assigned = MMUBITS(306, 1) != 0;
                        //ais->type19.spare      = MMUBITS(307, 5);
                        break;

#endif /* SAD_ENABLE_MESSAGE_19 */ 

#ifdef SAD_ENABLE_MESSAGE_9
                        /**
                         * message 9
                         */
                    case 9: /* Standard SAR Aircraft Position Report */
#ifdef SAD_STRICT_DECODE              
                        if (ais_context.bitlen != 168) {
                            SADERRM(MESSAGE9_PAYLOAD_NOT168);
                        }
#endif                 
                        ais->type9.alt = MMUBITS(38, 12);
                        ais->type9.speed = MMUBITS(50, 10);
                        ais->type9.accuracy = (bool) MMUBITS(60, 1);
                        ais->type9.lon = MMSBITS(61, 28);
                        ais->type9.lat = MMSBITS(89, 27);
                        ais->type9.course = MMUBITS(116, 12);
                        ais->type9.second = MMUBITS(128, 6);
                        ais->type9.regional = MMUBITS(134, 8);
                        ais->type9.dte = MMUBITS(142, 1);
                        //ais->type9.spare		= MMUBITS(143, 3);
                        ais->type9.assigned = MMUBITS(146, 1) != 0;
                        ais->type9.raim = MMUBITS(147, 1) != 0;
                        ais->type9.radio = MMUBITS(148, 19);
                        break;
#endif /* SAD_ENABLE_MESSAGE_9 */                         

                }
                ++(filter_->types[ais->type - 1]);
                
            }
        }

endline:
        if (mmerr) {
            ++filter_->errors;
            
            if(filter_->f_error_cb) filter_->f_error_cb(mmerr);
            free(mmerr);
            
        } else {
          
            
            /* drop duplicates */
            if (0 == strncmp(filter_->last_sentence, sentence->start, sentence->n)) {
              ++filter_->duplicates;
              
            }else
            {
                strncpy(filter_->last_sentence, sentence->start, sentence->n);
                filter_->last_sentence[sentence->n + 1] = '0';
                filter_->sentence = sentence;
                filter_->f_ais_cb(filter_);
            }
        }

    }
    return 0;
}

int sad_decode_file(sad_filter_t* filter_, const char* filename_) {

    FILE* f = fopen(filename_, "r");
    if (NULL == f)return 1;
    struct gps_packet_t pck;

    while (fgets((char*) pck.inbuffer, sizeof (pck.inbuffer), f) != NULL) {
        sad_decode_multiline(filter_, (char*) pck.inbuffer, strlen((char*) pck.inbuffer));
    }
    fclose(f);
    return 0;
}

#ifdef SAD_ENABLE_MESSAGE_1
#  define MMA1 "[x]"
#else
#  define MMA1 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_2
#  define MMA2 "[x]"
#else
#  define MMA2 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_3
#  define MMA3 "[x]"
#else
#  define MMA3 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_4
#  define MMA4 "[x]"
#else
#  define MMA4 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_5
#  define MMA5 "[x]"
#else
#  define MMA5 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_6
#  define MMA6 "[x]"
#else
#  define MMA6 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_7
#  define MMA7 "[x]"
#else
#  define MMA7 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_8
#  define MMA8 "[x]"
#else
#  define MMA8 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_9
#  define MMA9 "[x]"
#else
#  define MMA9 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_10
#  define MMA10 "[x]"
#else
#  define MMA10 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_11
#  define MMA11 "[x]"
#else
#  define MMA11 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_12
#  define MMA12 "[x]"
#else
#  define MMA12 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_13
#  define MMA13 "[x]"
#else
#  define MMA13 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_14
#  define MMA14 "[x]"
#else
#  define MMA14 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_15
#  define MMA15 "[x]"
#else
#  define MMA15 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_16
#  define MMA16 "[x]"
#else
#  define MMA16 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_17
#  define MMA17 "[x]"
#else
#  define MMA17 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_18
#  define MMA18 "[x]"
#else
#  define MMA18 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_19
#  define MMA19 "[x]"
#else
#  define MMA19 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_20
#  define MMA20 "[x]"
#else
#  define MMA20 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_21
#  define MMA21 "[x]"
#else
#  define MMA21 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_22
#  define MMA22 "[x]"
#else
#  define MMA22 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_23
#  define MMA23 "[x]"
#else
#  define MMA23 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_24
#  define MMA24 "[x]"
#else
#  define MMA24 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_25
#  define MMA25 "[x]"
#else
#  define MMA25 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_26
#  define MMA26 "[x]"
#else
#  define MMA26 "[ ]"
#endif
#ifdef SAD_ENABLE_MESSAGE_27
#  define MMA27 "[x]"
#else
#  define MMA27 "[ ]"
#endif


#define MF_FMT0 \
        "messages  :   %" PRIu64 "\n" \
        "frags     :   %" PRIu64 "\n" \
        "errors    :   %" PRIu64 "\n" \
        "duplicates:   %" PRIu64 "\n" \
        "type  1 " MMA1 ":  %" PRIu64 "\n" \
        "type  2 " MMA2 ":  %" PRIu64 "\n" \
        "type  3 " MMA3 ":  %" PRIu64 "\n" \
        "type  4 " MMA4 ":  %" PRIu64 "\n" \
        "type  5 " MMA5 ":  %" PRIu64 "\n" \
        "type  6 " MMA6 ":  %" PRIu64 "\n" \
        "type  7 " MMA7 ":  %" PRIu64 "\n" \
        "type  8 " MMA8 ":  %" PRIu64 "\n" \
        "type  9 " MMA9 ":  %" PRIu64 "\n" \
        "type 10 " MMA10 ":  %" PRIu64 "\n" \
        "type 11 " MMA11 ":  %" PRIu64 "\n" \
        "type 12 " MMA12 ":  %" PRIu64 "\n" \
        "type 13 " MMA13 ":  %" PRIu64 "\n" \
        "type 14 " MMA14 ":  %" PRIu64 "\n" \
        "type 15 " MMA15 ":  %" PRIu64 "\n" \
        "type 16 " MMA16 ":  %" PRIu64 "\n" \
        "type 17 " MMA17 ":  %" PRIu64 "\n" \
        "type 18 " MMA18 ":  %" PRIu64 "\n" \
        "type 19 " MMA19 ":  %" PRIu64 "\n" \
        "type 20 " MMA20 ":  %" PRIu64 "\n" \
        "type 21 " MMA21 ":  %" PRIu64 "\n" \
        "type 22 " MMA22 ":  %" PRIu64 "\n" \
        "type 23 " MMA23 ":  %" PRIu64 "\n" \
        "type 24 " MMA24 ":  %" PRIu64 "\n" \
        "type 25 " MMA25 ":  %" PRIu64 "\n" \
        "type 26 " MMA26 ":  %" PRIu64 "\n" \
        "type 27 " MMA27 ":  %" PRIu64 "\n" 

int sad_stats_string(char ** response_string, sad_filter_t* filter) {

#define MF_FILTER(TYPE) filter->types[TYPE] 
    return asprintf(response_string, MF_FMT0,
            filter->sentences,
            filter->frags,
            filter->errors,
            filter->duplicates,
            MF_FILTER(0), MF_FILTER(1), MF_FILTER(2), MF_FILTER(3),
            MF_FILTER(4), MF_FILTER(5), MF_FILTER(6), MF_FILTER(7),
            MF_FILTER(8), MF_FILTER(9),
            MF_FILTER(10), MF_FILTER(11), MF_FILTER(12),
            MF_FILTER(13), MF_FILTER(14), MF_FILTER(15), MF_FILTER(16),
            MF_FILTER(17), MF_FILTER(18), MF_FILTER(19),
            MF_FILTER(20), MF_FILTER(21), MF_FILTER(22),
            MF_FILTER(23), MF_FILTER(24), MF_FILTER(25), MF_FILTER(26));

}




