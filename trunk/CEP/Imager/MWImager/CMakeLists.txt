# $Id$

lofar_package(MWImager 0.1 DEPENDS Common MWCommon)

lofar_find_package(Casacore COMPONENTS images REQUIRED)

add_subdirectory(include/MWImager)
add_subdirectory(src)
add_subdirectory(test)
