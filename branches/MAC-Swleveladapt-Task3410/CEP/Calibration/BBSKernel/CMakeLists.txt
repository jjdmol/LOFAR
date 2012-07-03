# $Id$

lofar_package(BBSKernel 1.0 DEPENDS Blob Common ParmDB ElementResponse)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS 
  casa measures ms scimath tables images)

# Uncomment to check for unsafe conversions (gcc), for example conversion of
# size_t to unsigned int (truncation).
#add_definitions(-Wconversion)

# Uncomment to enable timing of (classes of) expression tree nodes. NB. Only
# enable if necessary, timing incurs an overhead of its own.
#add_definitions(-DLOFAR_BBSKERNEL_EXPR_TIMER)

add_subdirectory(include/BBSKernel)
add_subdirectory(src)
add_subdirectory(test)
