# $Id$

lofar_package(Appl_Controller 2.0 DEPENDS Common MACIO ALC PLC)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_subdirectory(src)

