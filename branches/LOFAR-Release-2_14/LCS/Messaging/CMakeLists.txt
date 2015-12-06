# $Id: CMakeLists.txt 1455 2015-08-18 07:31:18Z loose $

lofar_package(Messaging 1.0 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(QPID REQUIRED)
add_subdirectory(include/Messaging)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(python)
