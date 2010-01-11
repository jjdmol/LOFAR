# $Id$

lofar_package(APLCommon 3.1 DEPENDS Common ApplCommon GCFTM MACIO)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS regex)

add_definitions(-DBOOST_DISABLE_THREADS)

add_subdirectory(include/APL/APLCommon)
add_subdirectory(src)
add_subdirectory(test)
