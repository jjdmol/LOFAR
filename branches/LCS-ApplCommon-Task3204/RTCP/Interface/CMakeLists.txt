# $Id$

lofar_package(Interface 1.0 DEPENDS ApplCommon Common Interface Stream)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(Valgrind)

if(USE_VALGRIND)
  add_definitions(-DUSE_VALGRIND)
endif(USE_VALGRIND)  

## ---------------------------------------------------------------------------
## Enable BGP specific assembler.
## Use the BGP assembler also for linking C/C++ programs.
##
## NOTE: Maybe this should be moved to the variants file. That requires that
## CN and ION are no longer variants; we then need two compiler definitions:
## BGPCN and BGPION, which is a cleaner solution anyway.
## ---------------------------------------------------------------------------
enable_language(ASM-BGP)
if(LOFAR_COMPILER_SUITE STREQUAL "BGPCN")
  set(CMAKE_C_LINK_EXECUTABLE ${CMAKE_ASM-BGP_LINK_EXECUTABLE})
  set(CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_ASM-BGP_LINK_EXECUTABLE})
endif(LOFAR_COMPILER_SUITE STREQUAL "BGPCN")

add_subdirectory(include/Interface)
add_subdirectory(src)
add_subdirectory(test)
