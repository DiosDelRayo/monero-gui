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
add_subdirectory(framestate)

if(NOT DEFINED USE_EXTERNAL_QRENCODE OR NOT USE_EXTERNAL_QRENCODE)
    add_subdirectory(qrencode)
endif()

if (DEFINED UROTS_WITH_QTQUICK AND UROTS_WITH_QTQUICK)
    add_subdirectory(qtquick)
    # add_subdirectory(components)
    if (QT_VERSION_MAJOR EQUAL 6)
        qt6_add_resources(RESOURCES qml.qrc)
    else()
        qt5_add_resources(RESOURCES qml.qrc)
    endif()
endif()
