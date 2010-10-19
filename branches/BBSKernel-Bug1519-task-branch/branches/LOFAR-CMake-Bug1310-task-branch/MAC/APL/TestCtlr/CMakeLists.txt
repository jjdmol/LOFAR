# $Id$

lofar_package(TestCtlr 3.1 DEPENDS Common APLCommon MACIO GCFTM)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)

add_definitions(-DBOOST_DISABLE_THREADS)

add_subdirectory(src)
