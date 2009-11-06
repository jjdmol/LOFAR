# $Id$

lofar_package(MWImager 0.1 DEPENDS Common MWCommon)

lofar_find_package(Casacore COMPONENTS images REQUIRED)
lofar_find_package(HDF5)
lofar_find_package(Wcslib REQUIRED)
lofar_find_package(Cfitsio REQUIRED)
lofar_find_package(Lapack REQUIRED)
lofar_find_package(Blas REQUIRED)

add_subdirectory(include/MWImager)
add_subdirectory(src)
add_subdirectory(test)
