# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(BBSControl 1.0 DEPENDS BBSKernel Blob Common LMWCommon ParmDB PLC Transport)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS casa measures)

option(USE_PQXX "Enable BBS distributed calibration (using a postgres database as shared memory, requires pqxx)" OFF)
if(USE_PQXX)
  lofar_find_package(PQXX REQUIRED)
else(USE_PQXX)
  set(HAVE_PQXX OFF)
endif(USE_PQXX)

add_subdirectory(include/BBSControl)
add_subdirectory(src)
#add_subdirectory(test)    # test currently do NOT compile!
add_subdirectory(scripts)
