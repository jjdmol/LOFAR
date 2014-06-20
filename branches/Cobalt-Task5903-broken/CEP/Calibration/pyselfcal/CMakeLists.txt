# $Id: CMakeLists.txt 28302 2014-02-10 14:46:21Z amesfoort $

lofar_package(pyselfcal 1.0 DEPENDS BBSControl BBSKernel DPPP LofarFT PyBDSM)

include(LofarFindPackage)
lofar_find_package(Python 2.6 REQUIRED)
lofar_find_package(Numpy REQUIRED)
lofar_find_package(Pyrap REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
