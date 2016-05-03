# $Id$

lofar_package(MessageBus 1.0 DEPENDS Common pyparameterset PyCommon)

include(LofarFindPackage)
lofar_find_package(QPID)
lofar_find_package(LibXMLxx)
lofar_find_package(UnitTest++)

add_subdirectory(include/MessageBus)
add_subdirectory(src)
add_subdirectory(test)
