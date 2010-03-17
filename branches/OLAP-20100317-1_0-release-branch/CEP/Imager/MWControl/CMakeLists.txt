# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(MWControl 0.1 DEPENDS Common Blob MWCommon ParmDB BBSKernel)

add_subdirectory(include/MWControl)
add_subdirectory(src)
add_subdirectory(test)
