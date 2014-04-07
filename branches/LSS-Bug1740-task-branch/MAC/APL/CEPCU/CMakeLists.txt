# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(CEPCU 1.0 DEPENDS Common ALC PLC ApplCommon MACIO GCFTM GCFRTDB APLCommon)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)

add_subdirectory(src)
add_subdirectory(test/ApplController)   # Tests currently do NOT compile 
