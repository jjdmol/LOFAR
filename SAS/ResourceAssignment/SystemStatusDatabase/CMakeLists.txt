# $Id: CMakeLists.txt 33417 2016-01-28 07:37:45Z schaap $

lofar_package(SystemStatusDatabase 1.0 DEPENDS PyMessaging PyCommon)

lofar_find_package(Python 2.6 REQUIRED)
include(PythonInstall)

set(_py_files
  __init__.py
  ssdb.py
)

python_install(${_py_files} DESTINATION lofar/sas/systemstatus/database)

# add_subdirectory(test)
