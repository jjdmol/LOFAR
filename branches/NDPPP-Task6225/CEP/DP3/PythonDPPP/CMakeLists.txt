# $Id: CMakeLists.txt 27640 2013-12-04 08:02:49Z diepen $

lofar_package(PythonDPPP 0.1 DEPENDS DPPP)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS python)
lofar_find_package(Python 2.6 REQUIRED)

add_subdirectory(include/PythonDPPP)
add_subdirectory(src)
#add_subdirectory(test)
