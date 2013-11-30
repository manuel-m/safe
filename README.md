safe
====

simple AIS filter embeddable

External libraries have been included (lua,libuv,http-parser)


build instructions
==================

You will need libuv.a and liblua.a to build.
- libuv.a   => cd libs/libuv && ./buildit
- liblua.a  => cd libs/lua-5.2.2 && make linux
- ./configure && make
 


