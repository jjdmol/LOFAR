# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(Log_Client 1.0 DEPENDS Common ApplCommon GCFTM MACIO)

include(LofarFindPackage)
lofar_find_package(Log4Cplus REQUIRED)

add_definitions(-DBOOST_DISABLE_THREADS)

add_subdirectory(src)
