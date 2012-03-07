# $Id: CMakeLists.txt 18231 2011-06-09 14:46:14Z duscha $

lofar_package(BBSTools 1.0 DEPENDS Common)

#include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS 
  ms casa coordinates tables images measures)

lofar_find_package(Casarest REQUIRED COMPONENTS
  synthesis)


# Uncomment to check for unsafe conversions (gcc), for example conversion of
# size_t to unsigned int (truncation).
#add_definitions(-Wconversion)

add_subdirectory(include/BBSTools)
add_subdirectory(scripts)
add_subdirectory(src)
add_subdirectory(test)
