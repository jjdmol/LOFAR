# $Id$

lofar_package(ResourceAssignmentServices 1.0 DEPENDS PyMessaging PyCommon pyparameterset)

include(PythonInstall)

add_subdirectory(src)
add_subdirectory(test)
