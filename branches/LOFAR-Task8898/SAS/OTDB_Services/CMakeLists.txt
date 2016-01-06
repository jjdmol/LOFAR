# $Id$

lofar_package(OTDB_Services 1.0 DEPENDS PyMessaging OTDB)

include(PythonInstall)

lofar_add_bin_scripts(
  TreeService.py
  TreeStatusEvents.py
)

# supervisord config files
install(FILES
  TreeService.ini
  TreeStatusEvents.ini
  OTDB_Services.ini
  DESTIONATION etc/supervisord.d)


add_subdirectory(test)

