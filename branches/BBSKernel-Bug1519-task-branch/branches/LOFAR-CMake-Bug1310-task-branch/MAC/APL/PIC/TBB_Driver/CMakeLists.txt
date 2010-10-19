# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(TBB_Driver 6.0 DEPENDS Common TestSuite GCFTM MACIO RTCCommon TBB_Protocol)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Blitz REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
