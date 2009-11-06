# $Id$

lofar_package(Pipeline 0.1 DEPENDS pytools)

include(LofarFindPackage)
lofar_find_package(Python REQUIRED)

add_subdirectory(Core)
add_subdirectory(library)
