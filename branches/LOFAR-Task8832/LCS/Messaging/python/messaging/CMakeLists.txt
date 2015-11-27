# $Id$

lofar_package(PyMessaging 1.0 DEPENDS PyCommon)

include(PythonInstall)

set(_py_files
  __init__.py
  exceptions.py
  messagebus.py
  messages.py
  RPC.py
  Service.py
)

python_install(${_py_files} DESTINATION lofar/messaging)

add_subdirectory(test)
