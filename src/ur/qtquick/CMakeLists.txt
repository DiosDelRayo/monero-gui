set(URREGISTER_SOURCE
    UrRegister.cpp
)
set(URREGISTER_HEADERS
    UrRegister.h
)

add_library(urregister STATIC ${URREGISTER_SOURCE} ${URREGISTER_HEADERS})
target_include_directories(urregister PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR})
