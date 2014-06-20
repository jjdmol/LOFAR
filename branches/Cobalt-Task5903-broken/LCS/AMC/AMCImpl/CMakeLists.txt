# $Id$

lofar_package(AMCImpl 2.0 DEPENDS Common Transport AMCBase)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS measures REQUIRED)

add_subdirectory(include/AMCImpl)
add_subdirectory(src)
add_subdirectory(test)
