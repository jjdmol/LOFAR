# $Id$

#lofar_package(OTDB_Services 1.0 DEPENDS PyMessaging OTDB)

include(PythonInstall)

lofar_add_bin_scripts(
  TreeService.py
  TreeStatusEvents.py
)


add_subdirectory(test)

