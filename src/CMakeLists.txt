add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(plugins)

# Main executable
add_executable(FlashUp
    main.cpp
)

target_link_libraries(FlashUp
    PRIVATE
    flashup_core
    flashup_gui
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Quick
    Qt::QuickControls2
) 