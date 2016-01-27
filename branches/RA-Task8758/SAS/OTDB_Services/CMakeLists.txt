# $Id$

#lofar_package(OTDB_Services 1.0 DEPENDS PyMessaging OTDB)

lofar_find_package(Python 2.6 REQUIRED)
include(PythonInstall)

lofar_add_bin_scripts(
  TreeService.py
  TreeStatusEvents.py
)

python_install(OTDBBusListener.py
    DESTINATION lofar/sas/otdb)

add_subdirectory(test)

