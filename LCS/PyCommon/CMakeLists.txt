# $Id: CMakeLists.txt 720 2014-12-08 16:29:33Z loose $

lofar_package(PyCommon 1.0)

lofar_find_package(Python 2.6 REQUIRED)
include(PythonInstall)

set(_py_files
  __init__.py
  dbcredentials.py
  factory.py
  methodtrigger.py
  util.py
  postgres.py
  datetimeutils.py)

python_install(${_py_files} DESTINATION lofar/common)

add_subdirectory(test)
