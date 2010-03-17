# $Id$

lofar_package(VHECR 1.0 DEPENDS Common )

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)

##add_definitions(-DBOOST_DISABLE_THREADS)

add_subdirectory(include/VHECR)
add_subdirectory(src)
add_subdirectory(test)
