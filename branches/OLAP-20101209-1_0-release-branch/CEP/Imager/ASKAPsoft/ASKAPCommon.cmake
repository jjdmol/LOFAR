# - Common CMake code for the ASKAP software.

# $Id$

# Because the ASKAP sources do not include lofar_config.h, we set the
# necessary preprocessor variables here.
if(CMAKE_DL_LIBS)
  add_definitions(-DHAVE_DLOPEN)
endif(CMAKE_DL_LIBS)

if(HAVE_AIPSPP)
  add_definitions(-DHAVE_AIPSPP)
endif(HAVE_AIPSPP)

if(HAVE_BOOST)
  add_definitions(-DHAVE_BOOST)
endif(HAVE_BOOST)

if(HAVE_LOG4CXX)
  add_definitions(-DHAVE_LOG4CXX)
endif(HAVE_LOG4CXX)

if(LOFAR_BUILD_VARIANT MATCHES "^DEBUG$")
  add_definitions(-DASKAP_DEBUG)
endif(LOFAR_BUILD_VARIANT MATCHES "^DEBUG$")

# Create a separate directory for the symlinks to the ASKAP header files, and
# add this directory to the -I path.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/ASKAPsoft)
include_directories(${CMAKE_BINARY_DIR}/include/ASKAPsoft)
