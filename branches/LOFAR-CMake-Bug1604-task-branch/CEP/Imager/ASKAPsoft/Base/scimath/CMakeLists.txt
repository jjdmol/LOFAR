# $Id$

lofar_package(scimath DEPENDS askap Blob)

lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS 
  casa coordinates lattices measures scimath tables)
lofar_find_package(GSL REQUIRED)

# Because the ASKAP sources do not include lofar_config.h, we have to set the
# necessary preprocessor variables here manually.
if(HAVE_AIPSPP)
  add_definitions(-DHAVE_AIPSPP)
endif(HAVE_AIPSPP)
if(HAVE_BOOST)
  add_definitions(-DHAVE_BOOST)
endif(HAVE_BOOST)

add_subdirectory(src)
