# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/QT-ChatAdminServerGUI_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/QT-ChatAdminServerGUI_autogen.dir/ParseCache.txt"
  "QT-ChatAdminServerGUI_autogen"
  )
endif()
