# $Id$

# Online_OutputProc is needed for tests on cbt nodes
# APLCommon is needed for the `swlevel' script
lofar_package(Online_Cobalt DEPENDS Cobalt Online_OutputProc APLCommon PVSSGateway_Stub)

add_subdirectory(test)
