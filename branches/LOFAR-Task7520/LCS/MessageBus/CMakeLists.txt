# $Id$

lofar_package(MessageBus 1.0 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(QPID)
lofar_find_package(LibXml2)
lofar_find_package(UnitTest++)

if(HAVE_LIBXML2)
  add_definitions(-DHAVE_LIBXML2)
endif(HAVE_LIBXML2)

add_subdirectory(include/MessageBus)
add_subdirectory(src)
add_subdirectory(test)
