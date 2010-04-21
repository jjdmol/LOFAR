# $Id$

lofar_package(AOFlagger 1.0 DEPENDS LofarStMan)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-2.4)

include(LofarFindPackage)
lofar_find_package(LibXml2 REQUIRED)
lofar_find_package(PNG REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS ms tables)
lofar_find_package(Boost REQUIRED COMPONENTS date_time thread filesystem)
lofar_find_package(FFTW3 REQUIRED)
lofar_find_package(GSL REQUIRED)
lofar_find_package(CFITSIO REQUIRED)

add_subdirectory(include/AOFlagger)
add_subdirectory(src)
