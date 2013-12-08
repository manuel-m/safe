safe
====

simple AIS filter embeddable

External libraries sources have been included (lua,libuv,http-parser)
- libuv.a and liblua.a are built with bootstrap 
- some sources are generated using homemade lua code generator mmsdk.lua + mss_filter.lua => mss_filter_config.[ch]


build instructions
==================
./bootstrap && ./configure && make release


usage
=====

mss_filter ./mss_config.conf
