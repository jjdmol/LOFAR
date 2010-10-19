# $Id$

lofar_package(Common 3.1)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Readline)

add_subdirectory(include/Common)
add_subdirectory(src)
add_subdirectory(test)
