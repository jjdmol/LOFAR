# $Id$

lofar_package(LofarGridder 0.1 DEPENDS MWImager)

lofar_find_package(AskapSoft REQUIRED)
lofar_find_package(Casacore COMPONENTS images REQUIRED)

add_subdirectory(include/LofarGridder)
add_subdirectory(src)
add_subdirectory(test)
