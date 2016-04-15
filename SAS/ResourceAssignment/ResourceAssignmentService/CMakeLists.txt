# $Id$

lofar_package(ResourceAssignmentService 1.0 DEPENDS PyMessaging ResourceAssignmentDatabase)

lofar_find_package(Python 2.6 REQUIRED)
include(PythonInstall)

set(_py_files
  __init__.py
  config.py
  rpc.py
  service.py
)

python_install(${_py_files} DESTINATION lofar/sas/resourceassignment/resourceassignmentservice)

lofar_add_bin_scripts(radbservice radbclient)

# supervisord config files
install(FILES
  radbservice.ini
  DESTINATION etc/supervisord.d)

add_subdirectory(test)
