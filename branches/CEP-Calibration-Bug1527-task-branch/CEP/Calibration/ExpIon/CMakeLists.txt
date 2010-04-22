# $Id: CMakeLists.txt 14609 2009-12-07 09:10:48Z loose $

# Do not split the following line, otherwise makeversion will fail!
lofar_package(ExpIon 1.0 DEPENDS pyparameterset pyparmdb)

include(LofarFindPackage)
lofar_find_package(Python REQUIRED)
lofar_find_package(Pyrap REQUIRED)

add_subdirectory(src)
