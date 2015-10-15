# $Id: CMakeLists.txt 720 2014-12-08 16:29:33Z loose $

lofar_package(PyCommon 1.0)

include(PythonInstall)

#add_subdirectory(test)

set(_py_files
  factory.py
  util.py)

python_install(${_py_files} DESTINATION lofar/common)
