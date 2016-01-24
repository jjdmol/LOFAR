# $Id$

lofar_package(Scheduler 1.0)

# Search for Qt4 and setup CMake environment for it
find_package(Qt4 COMPONENTS QtCore QtGui QtSql QtTest REQUIRED)
include(${QT_USE_FILE})

# Add current source- and binary directory to include path
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(src)
add_subdirectory(test)
