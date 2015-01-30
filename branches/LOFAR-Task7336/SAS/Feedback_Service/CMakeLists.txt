# $Id: CMakeLists.txt 23213 2012-12-07 13:00:09Z loose $

lofar_package(FeedbackService 1.0 DEPENDS Common OTDB)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(PQXX REQUIRED)

add_subdirectory(src)
#add_subdirectory(test)

