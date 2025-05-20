set(SOURCES
    flashupcore.cpp
    firmwarepackage.cpp
    deviceinterface.cpp
    updatejob.cpp
    cryptoutils.cpp
)

set(HEADERS
    flashupcore.h
    firmwarepackage.h
    deviceinterface.h
    updatejob.h
    cryptoutils.h
)

add_library(flashup_core STATIC
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(flashup_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(flashup_core
    PRIVATE
    Qt::Core
    Qt::Network
) 