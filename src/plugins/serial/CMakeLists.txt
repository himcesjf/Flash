set(SOURCES
    serialdevice.cpp
)

set(HEADERS
    serialdevice.h
)

add_library(flashup_serial_plugin STATIC
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(flashup_serial_plugin
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(flashup_serial_plugin
    PRIVATE
    flashup_core
    Qt::Core
    Qt::SerialPort
) 