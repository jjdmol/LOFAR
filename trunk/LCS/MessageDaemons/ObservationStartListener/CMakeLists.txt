# $Id$

lofar_package(ObservationStartListener 1.0 DEPENDS MessageBus pyparameterset)

include(LofarFindPackage)
lofar_find_package(QPID)
lofar_find_package(LibXMLxx)

add_subdirectory(etc)
add_subdirectory(src)
add_subdirectory(test)
