set(SOURCES
    flashupgui.cpp
    logmodel.cpp
)

set(HEADERS
    flashupgui.h
    logmodel.h
)

set(QML_SOURCES
    ../resources/qml/main.qml
    ../resources/qml/DeviceList.qml
    ../resources/qml/FirmwareInfo.qml
    ../resources/qml/LogView.qml
    ../resources/qml/UpdatePanel.qml
)

qt_add_resources(QRC_SOURCES
    ../resources/resources.qrc
)

add_library(flashup_gui STATIC
    ${SOURCES}
    ${HEADERS}
    ${QRC_SOURCES}
)

target_include_directories(flashup_gui
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(flashup_gui
    PRIVATE
    flashup_core
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Quick
    Qt::QuickControls2
) 