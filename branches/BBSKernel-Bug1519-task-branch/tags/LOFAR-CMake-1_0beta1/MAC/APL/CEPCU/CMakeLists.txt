# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(CEPCU 3.1 DEPENDS Common ApplCommon ALC PLC MACIO GCFTM GCFRTDB APLCommon)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)

add_subdirectory(src)
add_subdirectory(test/ApplController)   # Tests currently do NOT compile 
