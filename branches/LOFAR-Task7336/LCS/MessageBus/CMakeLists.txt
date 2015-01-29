# $Id$

lofar_package(MessageBus 1.0 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(QPID)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)
