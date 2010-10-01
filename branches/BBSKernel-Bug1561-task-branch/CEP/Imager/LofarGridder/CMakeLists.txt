# $Id$

#lofar_package(LofarGridder 0.1 DEPENDS MWImager)
#lofar_package(LofarGridder 0.1 DEPENDS BBSKernel)

lofar_find_package(AskapSoft REQUIRED)
lofar_find_package(LOFAR COMPONENTS bbskernel parmdb mwcommon blob common REQUIRED)
lofar_find_package(Casacore COMPONENTS ms images REQUIRED)
lofar_find_package(Boost REQUIRED)

add_subdirectory(include/LofarGridder)
add_subdirectory(src)
add_subdirectory(test)
