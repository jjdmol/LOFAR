# $Id: CMakeLists.txt 30355 2014-11-04 13:46:05Z loose $

lofar_package(CleanupTool 1.0)

# Search for Qt4 and setup CMake environment for it
find_package(Qt4 COMPONENTS QtCore QtGui QtSql QtTest REQUIRED)
include(${QT_USE_FILE})

# Add current source- and binary directory to include path
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(src)

