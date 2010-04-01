#CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#PROJECT(detectrfi)

FIND_PACKAGE(PkgConfig)

PKG_CHECK_MODULES(GTKMM gtkmm-2.4)
#Now the variables GTKMM_INCLUDE_DIRS, GTKMM_LIBRARY_DIRS and GTKMM_LIBRARIES are
#defined.

pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

lofar_package(AOFlagger 1.0 DEPENDS LofarStMan)
include(LofarFindPackage)
lofar_find_package(Casacore REQUIRED COMPONENTS casa tables scimath scimath_f measures)
lofar_find_package(Boost REQUIRED COMPONENTS date_time thread filesystem system)
lofar_find_package(FFTW3 REQUIRED)
lofar_find_package(LAPACK REQUIRED)
lofar_find_package(CPPLapack REQUIRED)
lofar_find_package(BLAS REQUIRED)
lofar_find_package(PTHREADS REQUIRED)
lofar_find_package(GSL REQUIRED)

#want the full compiler output?
#set(CMAKE_VERBOSE_MAKEFILE ON)

#add definitions, compiler switches, etc.
add_definitions(-Wall -O2)

add_subdirectory(include/AOFlagger)
add_subdirectory(src)
