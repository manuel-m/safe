#ifndef _BR_PARSE_H
#define	_BR_PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

 typedef struct sub0_substring_s {
        const char* start;
        const char* end;
        size_t n;
    } sub0_substring_t;

    typedef struct sub0_line_s {
        const char* p;
        const char* start;
        const char* sub_start;
        int n;
        const char* end;
        sub0_substring_t sub;
        char sep;
    } sub0_line_t;

    int sub0_line_prepare(const char* _start, size_t _n, char _sep, sub0_line_t* _io);

    sub0_substring_t* sub0_line_next_substring(sub0_line_t* _io);
    
    
#ifdef __cplusplus
}
#endif

#endif /* _BR_PARSE_H */
