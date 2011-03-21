# $Id$

lofar_package(Pipeline-MAC 1.0 DEPENDS APLCommon MACIO)

lofar_find_package(Boost REQUIRED COMPONENTS python)
lofar_find_package(Python REQUIRED)

add_subdirectory(src)
add_subdirectory(ep)

#add_subdirectory(test)
