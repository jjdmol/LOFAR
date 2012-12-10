# $Id$

lofar_package(IONProc 1.0
  DEPENDS Common ApplCommon Stream Interface FCNP)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Libssh2)
lofar_find_package(OpenSSL)
lofar_find_package(Valgrind)
lofar_find_package(Casacore COMPONENTS measures REQUIRED)

if(USE_VALGRIND)
  add_definitions(-DUSE_VALGRIND)
endif(USE_VALGRIND)  

enable_language(ASM-BGP)

add_subdirectory(src)
add_subdirectory(test)
