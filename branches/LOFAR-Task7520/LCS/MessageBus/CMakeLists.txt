# $Id$

lofar_package(MessageBus 1.0 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(QPID)
lofar_find_package(LibXml++)
lofar_find_package(UnitTest++)

if(HAVE_LIBXML2)
  add_definitions(-DHAVE_LIBXML)
endif(HAVE_LIBXML)

add_subdirectory(include/MessageBus)
add_subdirectory(src)
add_subdirectory(test)
