
spec2def(kdcom.dll kdcom.spec ADD_IMPORTLIB)

list(APPEND SOURCE
    kdcom.c
    kddll.c
    kdserial.c
    kddll.h)

add_library(kdcom SHARED
    ${SOURCE}
    kdcom.rc
    ${CMAKE_CURRENT_BINARY_DIR}/kdcom.def)

set_module_type(kdcom module IMAGEBASE 0x00010000)
set_subsystem(kdcom native)
add_importlibs(kdcom ntoskrnl hal)
target_link_libraries(kdcom cportlib)
add_dependencies(kdcom psdk bugcodes)
add_pch(kdcom kddll.h SOURCE)
add_cd_file(TARGET kdcom DESTINATION reactos/system32 NO_CAB FOR all)
