# $Id: CMakeLists.txt 21382 2012-06-29 11:26:52Z mol $

lofar_package(InputProc 1.0
  DEPENDS Common Stream Interface IONProc)

include(LofarFindPackage)
#lofar_find_package(Boost REQUIRED)
#lofar_find_package(Libssh2)
#lofar_find_package(Valgrind)
#lofar_find_package(Casacore COMPONENTS measures REQUIRED)

add_subdirectory(src)
#add_subdirectory(test)
