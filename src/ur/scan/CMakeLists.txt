project(liburscan)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(ZXing REQUIRED)

set(URSCAN_SOURCES
    QrScanThread.cpp
    ScanWidget.ui
    ScanWidget.cpp
)

set(URSCAN_HEADERS
    ScanResult.h
    QrScanThread.h
    ScanWidget.h
)
add_library(urscan STATIC ${URSCAN_SOURCES} ${URSCAN_HEADERS})
set_target_properties(urscan PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(urscan PROPERTIES CXX_STANDARD 17)

target_include_directories(urscan PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})
