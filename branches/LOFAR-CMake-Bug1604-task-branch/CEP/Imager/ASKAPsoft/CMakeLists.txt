# $Id$

# If this is a debug build, we need to set ASKAP_DEBUG
if(LOFAR_BUILD_VARIANT MATCHES "^DEBUG$")
  add_definitions(-DASKAP_DEBUG)
endif(LOFAR_BUILD_VARIANT MATCHES "^DEBUG$")

# Because the ASKAP sources do not include lofar_config.h, we have to set the
# necessary preprocessor variables here manually.
if(HAVE_LOG4CXX)
  add_definitions(-DHAVE_LOG4CXX)
endif(HAVE_LOG4CXX)

lofar_add_package(Base)
lofar_add_package(Components)
