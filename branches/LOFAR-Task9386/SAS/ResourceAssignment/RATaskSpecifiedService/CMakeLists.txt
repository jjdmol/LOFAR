# $Id$

lofar_package(RATaskSpecifiedService 1.0 DEPENDS PyMessaging PyCommon pyparameterset OTDB_Services)

include(PythonInstall)

add_subdirectory(lib)
add_subdirectory(bin)
add_subdirectory(test)
