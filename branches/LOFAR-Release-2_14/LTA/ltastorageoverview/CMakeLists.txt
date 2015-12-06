# $Id$

lofar_find_package(Python 2.6 REQUIRED)
lofar_package(ltastorageoverview 0.1 DEPENDS PyCommon)

include(PythonInstall)
set(USE_PYTHON_COMPILATION Off)

add_subdirectory(lib)
add_subdirectory(bin)
add_subdirectory(test)
