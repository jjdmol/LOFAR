# $Id$

lofar_package(MoMQueryService 1.0 DEPENDS PyMessaging)

include(PythonInstall)

set(_py_files
  __init__.py
  config.py
  momqueryservice.py
  momprojectdetailsquery.py
)

python_install(${_py_files} DESTINATION lofar/mom/momqueryservice)

add_subdirectory(test)
