project(libqrcode)

add_library(qrcode STATIC QrCode.cpp)
set_target_properties(qrcode PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(qrcode PROPERTIES CXX_STANDARD 17)

target_include_directories(qrcode PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR})
