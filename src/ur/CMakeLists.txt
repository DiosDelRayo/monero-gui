cmake_minimum_required(VERSION 3.10)
project(libur VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(USE_EXTERNAL_QRENCODE "Use an external qrencode library" OFF)

add_subdirectory(bcur)
add_subdirectory(display)
add_subdirectory(scan)
add_subdirectory(qrcode)

if(NOT USE_EXTERNAL_QRENCODE)
    add_subdirectory(qrencode)
endif()
