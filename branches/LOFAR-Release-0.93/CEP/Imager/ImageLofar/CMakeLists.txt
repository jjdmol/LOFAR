# $Id$

lofar_package(ImageLofar 0.1 DEPENDS Common)

lofar_find_package(Casacore REQUIRED COMPONENTS images)

add_subdirectory(include/ImageLofar)
add_subdirectory(src)
add_subdirectory(test)
