# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(RTDBCommon 3.1 DEPENDS Common GCFPVSS GCFRTDB GCFTM MACIO)

add_subdirectory(include/APL/RTDBCommon)
add_subdirectory(src)
add_subdirectory(test)
