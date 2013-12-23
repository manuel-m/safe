safe
====

simple AIS filter embeddable.

External libraries sources have been included (lua,libuv,http-parser)
- see libs/buildit scripts to generate liblua.a and libuv.a 
- some sources are generated using homemade lua code generator mmsdk.lua + ais_filter_config_api.lua => ais_filter_config.[ch]
- libuv needs gyp on linux

build instructions
==================

- linux cmake
-   ./bootstrap && ./configure && make release

- linux without cmake
-   ./bootstrap && ./buildit

- windows 
- see buildit_win script on windows (mingw expected in /c/Mingw)
- for convenience, win32/mss_filter32.exe is provided

- ./boostrap nolibs to avoid libuv.a liblua.a build

usage
=====

ais_filter /path/to/ais_filter.conf


TODO
======

- smarter config code generator (multilevel,less code)
- NMEA or timestamped NMEA output
- vessels server
- vessels history sharing
- http server using leafjs

