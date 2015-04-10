# $Id$

lofar_package(OutputProc 1.0 DEPENDS Common Stream CoInterface MSLofar LofarStMan MACIO MessageBus)

include(LofarFindPackage)

lofar_find_package(OpenMP REQUIRED)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore COMPONENTS casa ms tables REQUIRED)
lofar_find_package(DAL)  # REQUIRED for BF and TBB
if(DAL_FOUND)
  lofar_find_package(HDF5 REQUIRED)
endif(DAL_FOUND)
lofar_find_package(UnitTest++)

add_subdirectory(src)
add_subdirectory(test)
