# $Id$

lofar_package(ResourceAssignmentDatabase 1.0 DEPENDS PyMessaging PyCommon)

lofar_find_package(Python 2.6 REQUIRED)
include(PythonInstall)

set(_py_files
  __init__.py
  config.py
  radb.py
)

python_install(${_py_files} DESTINATION lofar/sas/resourceassignment/database)

# add_subdirectory(test)
