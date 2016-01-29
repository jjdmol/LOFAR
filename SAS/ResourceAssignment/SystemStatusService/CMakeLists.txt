# $Id: CMakeLists.txt $

lofar_package(SystemStatusService 1.0 DEPENDS PyMessaging)

lofar_find_package(Python 2.6 REQUIRED)
include(PythonInstall)

set(_py_files
  __init__.py
  SSDBQueryService.py
  SSDBrpc.py
)

python_install(${_py_files} DESTINATION lofar/sas/resourceassignment/systemstatus)

lofar_add_bin_scripts(ssdbservice)

# supervisord config files
install(FILES
  ssdbservice.ini
  DESTINATION etc/supervisord.d)

add_subdirectory(test)
