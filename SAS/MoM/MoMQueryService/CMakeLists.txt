# $Id$

lofar_package(MoMQueryService 1.0 DEPENDS PyMessaging)

include(PythonInstall)

set(_py_files
  __init__.py
  config.py
  momqueryservice.py
  momqueryrpc.py
)

python_install(${_py_files} DESTINATION lofar/mom/momqueryservice)

lofar_add_bin_scripts(momqueryservice momquery)

# supervisord config files
install(FILES
  momqueryservice.ini
  DESTINATION etc/supervisord.d)

add_subdirectory(test)
