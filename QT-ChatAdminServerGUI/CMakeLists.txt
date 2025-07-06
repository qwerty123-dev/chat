cmake_minimum_required(VERSION 3.19)
project(QT-ChatAdminServerGUI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()

qt_add_executable(QT-ChatAdminServerGUI
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    adminclient.h
    adminclient.cpp
    settings.h
    settings.cpp

    database.h
    database.cpp

)

target_link_libraries(QT-ChatAdminServerGUI
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Network
)

include(GNUInstallDirs)

install(TARGETS QT-ChatAdminServerGUI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QT-ChatAdminServerGUI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
