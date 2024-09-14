project(libqrencode)

# Add the qrencode source files
set(QRENCODE_SOURCES
    qrencode.c
    qrinput.c
    qrspec.c
    rsecc.c
    mask.c
    split.c
    bitstream.c
    mmask.c
    mqrspec.c
)

# Add the qrencode headers
set(QRENCODE_HEADERS
    qrencode_inner.h
    qrencode.h
    qrinput.h
    qrspec.h
    rsecc.h
    mask.h
    split.h
    bitstream.h
    mmask.h
    mqrspec.h
)

add_library(qrencode STATIC ${QRENCODE_SOURCES} ${QRENCODE_HEADERS})
target_compile_definitions(qrencode PRIVATE HAVE_CONFIG_H STATIC_IN_RELEASE=static)
