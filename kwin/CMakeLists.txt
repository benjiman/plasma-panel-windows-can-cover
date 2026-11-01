find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(KWin REQUIRED)
find_package(KF6Config REQUIRED)

add_library(kwin_windowscancover MODULE main.cpp windowscancover.cpp windowscancover.h)
set_target_properties(kwin_windowscancover PROPERTIES OUTPUT_NAME windowscancover PREFIX "")
target_link_libraries(kwin_windowscancover PRIVATE KWin::kwin KF6::ConfigCore)
install(TARGETS kwin_windowscancover DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt6/plugins/kwin/plugins)
