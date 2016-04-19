# $Id$

lofar_package(OTDBtoRATaskStatusPropagator 1.0 DEPENDS PyMessaging ResourceAssignmentService OTDB_Services)

lofar_find_package(Python 2.6 REQUIRED)
include(PythonInstall)

set(_py_files
  __init__.py
  propagator.py
)

python_install(${_py_files} DESTINATION lofar/sas/resourceassignment/otdbtorataskstatuspropagator)

lofar_add_bin_scripts(otdbtorataskstatuspropagator)

# supervisord config files
install(FILES
  otdbtorataskstatuspropagator.ini
  DESTINATION etc/supervisord.d)

