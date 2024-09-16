set(URREGISTER_SOURCE
    UrRegister.cpp
)
set(URREGISTER_HEADERS
    UrRegister.h
)
if (QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 COMPONENTS Core Gui Widgets MultimediaWidgets QUIET)
else()
    find_package(Qt5 COMPONENTS Core Gui Widgets MultimediaWidgets REQUIRED)
endif()

add_library(urregister STATIC ${URREGISTER_SOURCE} ${URREGISTER_HEADERS})
target_include_directories(urregister PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(urregister PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::MultimediaWidgets
)
