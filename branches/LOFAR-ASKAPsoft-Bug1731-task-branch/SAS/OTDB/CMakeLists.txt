# $Id$

lofar_package(OTDB 1.0 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(PQXX REQUIRED)

add_subdirectory(include/OTDB)
add_subdirectory(src)
add_subdirectory(test)
