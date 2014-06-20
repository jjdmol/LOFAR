# $Id$

lofar_package(Stream 0.1 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)

add_subdirectory(include/Stream)
add_subdirectory(src)
add_subdirectory(test)
