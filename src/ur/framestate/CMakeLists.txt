set(FRAMESTATE_SOURCE
    FrameState.cpp
)
set(FRAMESTATE_HEADERS
    FrameState.h
)

add_library(framestate STATIC ${FRAMESTATE_SOURCE} ${FRAMESTATE_HEADERS})
target_include_directories(framestate PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR})
