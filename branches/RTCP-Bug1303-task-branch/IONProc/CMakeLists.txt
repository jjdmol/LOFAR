# $Id$

lofar_package(IONProc 1.0
  DEPENDS Common Stream AMCBase AMCImpl PLC Interface FCNP Thread)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Valgrind)

if(USE_VALGRIND)
  add_definitions(-DUSE_VALGRIND)
endif(USE_VALGRIND)  

enable_language(ASM-BGP)

add_subdirectory(src)
add_subdirectory(test)
