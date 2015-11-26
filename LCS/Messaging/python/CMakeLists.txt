# $Id: CMakeLists.txt 1584 2015-10-02 12:10:14Z loose $

lofar_find_package(Python 2.7)
if(PYTHON_FOUND)
  lofar_add_package(PyMessaging messaging)
else()
  message(WARNING "Python module lofar.messaging will NOT be built")
endif()
