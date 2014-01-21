safe
====

simple AIS filter embeddable.

External libraries sources have been included (libconfig,libuv,http-parser)
- see libs/buildit scripts to generate libconfig.a and libuv.a 
- libuv needs gyp on linux

build instructions
==================

- linux cmake
-   ./bootstrap && ./configure && make release

- linux without cmake
-   ./bootstrap && ./buildit

- windows 
- see buildit_win script on windows (mingw expected in /c/Mingw)

- ./boostrap nolibs to avoid libuv.a liblua.a build

usage
=====

ais_filter /path/to/ais_filter.cfg
ais_buffer /path/to/ais_buffer.cfg

in course
=========

- vessels history (medium range buffer)

TODO
=====

- tcp client tools name resolution
- vessels history FT
- default config display
- vessels server
- http server 

