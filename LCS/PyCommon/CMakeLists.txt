# $Id: CMakeLists.txt 720 2014-12-08 16:29:33Z loose $

lofar_package(PyCommon 1.0)

lofar_find_package(Python 2.6 REQUIRED)
include(PythonInstall)

add_subdirectory(test)

set(_py_files
  dbcredentials.py
  factory.py
  util.py
  datetimeutils.py)

python_install(${_py_files} DESTINATION lofar/common)
