# $Id$

lofar_package(AOFlagger 1.0 DEPENDS LofarStMan Common LMWCommon)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-2.4>=2.18)
pkg_check_modules(SIGCXX sigc++-2.0)

include(LofarFindPackage)
lofar_find_package(GSL)
lofar_find_package(LibXml2 REQUIRED)
lofar_find_package(PNG REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS ms tables)
lofar_find_package(Boost REQUIRED COMPONENTS date_time thread filesystem system)
lofar_find_package(CFITSIO REQUIRED)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  find_library(RT_LIBRARY rt)
  list(APPEND LOFAR_EXTRA_LIBRARIES ${RT_LIBRARY})
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

add_subdirectory(include/AOFlagger)
add_subdirectory(src)
add_subdirectory(test)
