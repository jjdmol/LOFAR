# $Id$

lofar_package(IONProc 1.0
  DEPENDS Common Stream AMCBase AMCImpl PLC Interface FCNP)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)

enable_language(ASM-BGP)

add_subdirectory(src)
#add_subdirectory(test)    # Tests currently do NOT compile!
