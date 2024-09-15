set(QRCODE_SOURCE
    QrCode.cpp
)
set(QRCODE_HEADERS
    QrCode.h
    QrCode_p.h
)

add_library(qrcode STATIC ${QRCODE_SOURCE} ${QRCODE_HEADERS})
target_include_directories(qrcode PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR})
