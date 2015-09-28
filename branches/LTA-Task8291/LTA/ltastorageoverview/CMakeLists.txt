# $Id: CMakeLists.txt 32341 2015-08-28 11:59:26Z schaap $

lofar_package(ltastorageoverview 0.1)

include(PythonInstall)
set(USE_PYTHON_COMPILATION Off)

add_subdirectory(lib)
add_subdirectory(bin)
add_subdirectory(test)
