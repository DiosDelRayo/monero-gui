add_library(ur-encoder STATIC ur-encoder.cpp)
set_target_properties(ur-encoder PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(ur-encoder PROPERTIES CXX_STANDARD 17)

target_include_directories(ur-encoder PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR})

add_library(ur-decoder STATIC ur-decoder.cpp)
set_target_properties(ur-decoder PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(ur-decoder PROPERTIES CXX_STANDARD 17)

target_include_directories(ur-decoder PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR})

