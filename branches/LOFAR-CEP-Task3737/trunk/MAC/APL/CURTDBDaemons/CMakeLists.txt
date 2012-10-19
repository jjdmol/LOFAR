# $Id$

lofar_package(CURTDBDaemons 1.0 DEPENDS Common ApplCommon MACIO GCFTM GCFRTDB RTDBCommon APLCommon OTDB)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(PQXX REQUIRED)

add_definitions(-DBOOST_DISABLE_THREADS)

add_subdirectory(src)
add_subdirectory(test)
