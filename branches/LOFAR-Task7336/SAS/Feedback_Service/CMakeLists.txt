# $Id: CMakeLists.txt 23213 2012-12-07 13:00:09Z loose $

lofar_package(Feedback_Service 1.0 DEPENDS Common MACIO GCFTM OTDB MessageBus)
#lofar_package(CEPCU 1.0 DEPENDS Common ApplCommon MACIO GCFTM GCFRTDB APLCommon RTDBCommon OTDB Stream MessageBus)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(PQXX REQUIRED)

add_subdirectory(src)
#add_subdirectory(test)

