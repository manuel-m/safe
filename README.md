safe
====

simple AIS filter embeddable.

External libraries sources have been included (lua,libuv,http-parser)
- see libs/buildit scripts to generate liblua.a and libuv.a
- some sources are generated using homemade lua code generator mmsdk.lua + mss_filter.lua => mss_filter_config.[ch]


build instructions
==================

- linux
- ./bootstrap && ./configure && make release

- windows 
- see buildit_win script on windows (mingw expected in /c/Mingw)
- for convenience, win32/mss_filter32.exe is provided

usage
=====

mss_filter ./mss_config.conf
