# $Id$

lofar_package(jOTDB2 1.0 DEPENDS Common OTDB)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(PQXX REQUIRED)

add_subdirectory(include/jOTDB2)
add_subdirectory(src)
#add_subdirectory(test)
