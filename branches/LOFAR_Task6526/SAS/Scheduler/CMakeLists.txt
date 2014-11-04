# $Id$

lofar_package(Scheduler 1.0)

# Search for Qt4 and setup CMake environment for it
find_package(Qt4 COMPONENTS QtCore QtGui QtSql REQUIRED)
include(${QT_USE_FILE})

add_subdirectory(src)
