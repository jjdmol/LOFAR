# $Id$

lofar_package(MessageBus 1.0 DEPENDS Common pyparameterset)

include(LofarFindPackage)
lofar_find_package(QPID)

add_subdirectory(include/MessageBus)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(management)