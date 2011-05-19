# $Id$

lofar_package(CasaGridder 0.1 DEPENDS BBSKernel Common ParmDB)

lofar_find_package(Casarest REQUIRED COMPONENTS synthesis)
lofar_find_package(Casacore REQUIRED COMPONENTS images msfits)
lofar_find_package(Boost REQUIRED COMPONENTS thread)

# Needed for casarest
add_definitions(-DCASA_STANDALONE)

add_subdirectory(include/CasaGridder)
add_subdirectory(src)
add_subdirectory(test)
