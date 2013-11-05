# define LIBGDI32_A,LIBNTDLL_A,LIBIPHLPAPI_A,LIBWS2_32_A,LIBPSAPI_A
# add headers dir

MACRO(macro_use_mingw )

set( LIBMINGW_BASEPATH "${NSHARED_DEP}/mingw/${VMAJOR}.${VMINOR}" )

include_directories (${LIBMINGW_BASEPATH}/headers)

set ( LIBGDI32_A /c/MinGW/lib/libgdi32.a  )
set ( LIBNTDLL_A /c/MinGW/lib/libntdll.a  )
set ( LIBIPHLPAPI_A /c/MinGW/lib/libiphlpapi.a  )
set ( LIBWS2_32_A /c/MinGW/lib/libws2_32.a  )
set ( LIBPSAPI_A /c/MinGW/lib/libpsapi.a  )


ENDMACRO(macro_use_mingw)

