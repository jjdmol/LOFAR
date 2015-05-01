# $Id$

lofar_package(Pipeline-Recipes 0.1)

# The pipeline.cfg needs to know whether QPID is installed
include(LofarFindPackage)
lofar_find_package(QPID)

add_subdirectory(sip)
