include("/home/qwerty/QT-ChatClientGUI/build/Desktop-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/QT-ChatClientGUI-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE /home/qwerty/QT-ChatClientGUI/build/Desktop-Debug/QT-ChatClientGUI
    GENERATE_QT_CONF
)
