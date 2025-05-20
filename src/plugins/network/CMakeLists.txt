set(SOURCES
    networkdevice.cpp
)

set(HEADERS
    networkdevice.h
)

add_library(flashup_network_plugin STATIC
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(flashup_network_plugin
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(flashup_network_plugin
    PRIVATE
    flashup_core
    Qt::Core
    Qt::Network
) 