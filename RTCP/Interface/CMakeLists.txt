# $Id$

lofar_package(Interface 1.0 DEPENDS Common Stream ApplCommon)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)

add_subdirectory(include/Interface)
add_subdirectory(src)
add_subdirectory(test)
