cmake_minimum_required(VERSION 3.22)
project(plasma-windows-can-cover VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)

find_package(Qt6 6.6 REQUIRED COMPONENTS Gui Qml Quick)
find_package(LayerShellQt REQUIRED)
find_package(Plasma REQUIRED)

set(QML_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt6/qml" CACHE PATH "Where to install the QML module")
set(SHELL_ID "org.windowscancover.desktop" CACHE STRING "Id of the overlay shell package")
set(STOCK_SHELL_DIR "${CMAKE_INSTALL_FULL_DATADIR}/plasma/shells/org.kde.plasma.desktop" CACHE PATH "Stock Plasma desktop shell package")

qt_policy(SET QTP0001 NEW)
qt_add_qml_module(windowscancoverplugin
    URI org.windowscancover
    VERSION 1.0
    PLUGIN_TARGET windowscancoverplugin
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/org/windowscancover
    NO_GENERATE_PLUGIN_SOURCE
    CLASS_NAME WindowsCanCoverPlugin
    SOURCES
        src/plugin.cpp
        src/panelcover.cpp src/panelcover.h
        src/edgetrigger.cpp src/edgetrigger.h
)
target_include_directories(windowscancoverplugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(windowscancoverplugin PRIVATE Qt6::Gui Qt6::Qml LayerShellQt::Interface Plasma::Plasma)

install(TARGETS windowscancoverplugin DESTINATION ${QML_INSTALL_DIR}/org/windowscancover)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org/windowscancover/qmldir ${CMAKE_CURRENT_BINARY_DIR}/org/windowscancover/windowscancoverplugin.qmltypes DESTINATION ${QML_INSTALL_DIR}/org/windowscancover)

# Overlay shell package: only overrides what it needs, falls back to org.kde.plasma.desktop
configure_file(shell/metadata.json.in ${CMAKE_CURRENT_BINARY_DIR}/shell/metadata.json @ONLY)
configure_file(shell/contents/views/Panel.qml.in ${CMAKE_CURRENT_BINARY_DIR}/shell/contents/views/Panel.qml @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/shell/metadata.json DESTINATION ${CMAKE_INSTALL_DATADIR}/plasma/shells/${SHELL_ID})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/shell/contents/views/Panel.qml DESTINATION ${CMAKE_INSTALL_DATADIR}/plasma/shells/${SHELL_ID}/contents/views)

configure_file(bin/plasma-windows-can-cover.in ${CMAKE_CURRENT_BINARY_DIR}/bin/plasma-windows-can-cover @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/plasma-windows-can-cover DESTINATION ${CMAKE_INSTALL_BINDIR})
