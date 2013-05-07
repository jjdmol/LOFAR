# $Id: CMakeLists.txt 21382 2012-06-29 11:26:52Z mol $

lofar_package(InputProc 1.0
  DEPENDS ApplCommon Common Stream CoInterface)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(Casacore COMPONENTS casa measures)
lofar_find_package(OpenMP REQUIRED)
lofar_find_package(MPI REQUIRED)
lofar_find_package(UnitTest++)

add_subdirectory(src)
add_subdirectory(test)
