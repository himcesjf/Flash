cmake_minimum_required(VERSION 3.16)
project(FlashUp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt packages
find_package(Qt6 COMPONENTS Core Gui Widgets Quick QuickControls2 SerialPort Network Bluetooth REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets Quick QuickControls2 SerialPort Network Bluetooth REQUIRED)
endif()

# Include subdirectories
add_subdirectory(src)

# Testing
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install targets
install(TARGETS FlashUp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# CPack for installers
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VENDOR "FlashUp")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cross-platform firmware/OTA updater & diagnostics tool")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
include(CPack) 