
#include <string.h>
#include "sub0.h"

int sub0_line_prepare(const char* _start, size_t _n, char _sep, sub0_line_t* _io) {
    memset(_io, 0, sizeof (*_io));
    _io->sep = _sep;
    _io->start = _start;
    _io->p = _start;
    _io->end = _start + _n;
    return 0;
}

sub0_substring_t* sub0_line_next_substring(sub0_line_t* _io) {
    while (_io->p <= _io->end) {
        if ((*_io->p == _io->sep) || (_io->p == _io->end)) { /* sep */
            if (_io->sub_start) {
                _io->sub.start = _io->sub_start;
                _io->sub.end = _io->p - 1;
                _io->sub.n = (size_t) (_io->p - _io->sub_start);
            } else {
                _io->sub.n = 0;
            }
            ++_io->p;
            _io->sub_start = NULL;
            return &_io->sub;
        } else { /* no sep */
            if (NULL == _io->sub_start) {
                _io->sub_start = _io->p;
            }
        }
        ++_io->p;
    }
    return NULL;
}
