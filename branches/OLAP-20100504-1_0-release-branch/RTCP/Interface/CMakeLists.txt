# $Id$

lofar_package(Interface 1.0 DEPENDS Common Stream Thread)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(Valgrind)

if(USE_VALGRIND)
  add_definitions(-DUSE_VALGRIND)
endif(USE_VALGRIND)  

add_subdirectory(include/Interface)
add_subdirectory(src)
add_subdirectory(test)
