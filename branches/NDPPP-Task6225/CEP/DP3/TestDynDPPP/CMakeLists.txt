# $Id: CMakeLists.txt 27640 2013-12-04 08:02:49Z diepen $

lofar_package(TestDynDPPP 0.1 DEPENDS DPPP)

add_subdirectory(src)
add_subdirectory(test)
