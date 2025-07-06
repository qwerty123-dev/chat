cmake_minimum_required(VERSION 3.19)
project(QT-ChatClientGUI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui Network)

qt_standard_project_setup()

qt_add_executable(QT-ChatClientGUI
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(QT-ChatClientGUI
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Gui
        Qt6::Network
)

include(GNUInstallDirs)

install(TARGETS QT-ChatClientGUI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QT-ChatClientGUI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
