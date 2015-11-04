# $Id$

#lofar_package(OTDB_Services 1.0 DEPENDS PyMessaging OTDB)

include(PythonInstall)

set(_py_files
  __init__.py
  TreeService.py
  TreeStatusEvents.py
)

python_install(${_py_files} DESTINATION bin)

add_subdirectory(test)

