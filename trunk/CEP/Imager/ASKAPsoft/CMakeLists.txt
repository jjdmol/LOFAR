# $Id$

# ASKAPsoft requires log4cxx. Do not use lofar_find_package() here,
# to avoid conflicts when the rest of LOFAR is built with log4cplus.
find_package(Log4Cxx REQUIRED)
include_directories(${LOG4CXX_INCLUDE_DIRS})
list(APPEND LOFAR_EXTRA_LIBRARIES ${LOG4CXX_LIBRARIES})

lofar_add_package(ASKAP-Base Base)
lofar_add_package(ASKAP-Components Components)
