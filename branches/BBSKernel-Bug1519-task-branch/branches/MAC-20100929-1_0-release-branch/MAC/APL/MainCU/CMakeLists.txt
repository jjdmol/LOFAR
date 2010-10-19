# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(MainCU 1.0 DEPENDS Common OTDB MACIO GCFTM GCFPVSS GCFRTDB APLCommon RTDBCommon ApplCommon)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)

add_definitions(-DBOOST_DISABLE_THREADS)

add_subdirectory(src)
