# $Id$

lofar_package(GSM 1.0)

include(LofarFindPackage)
lofar_find_package(Python 2.6 REQUIRED)

add_subdirectory(src)
#add_subdirectory(db)
add_subdirectory(test)
