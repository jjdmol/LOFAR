# $Id$

lofar_package(VHECR 1.0 DEPENDS Common ApplCommon APLCommon)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time program_options)

##add_definitions(-DBOOST_DISABLE_THREADS)

add_subdirectory(include/VHECR)
add_subdirectory(src)
add_subdirectory(test)
