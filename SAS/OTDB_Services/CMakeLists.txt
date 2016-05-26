# $Id$

lofar_package(OTDB_Services 1.0 DEPENDS PyMessaging)

lofar_find_package(Python 2.6 REQUIRED)
include(PythonInstall)

lofar_add_bin_scripts(
  getParset.py
  setStatus.py
  TreeService.py
  TreeStatusEvents.py
)

set(_py_files
  config.py
  otdbrpc.py
  OTDBBusListener.py
)

# supervisord config files
install(FILES
  TreeService.ini
  TreeStatusEvents.ini
  OTDB_Services.ini
  DESTINATION etc/supervisord.d)

python_install(${_py_files} DESTINATION lofar/sas/otdb)

add_subdirectory(test)

