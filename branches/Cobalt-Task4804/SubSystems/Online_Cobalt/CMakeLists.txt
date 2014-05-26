# $Id$

# Online_OutputProc is needed for tests on cbt nodes
# APLCommon is needed for the `swlevel' script
lofar_package(Online_Cobalt DEPENDS Cobalt Online_OutputProc APLCommon)

add_subdirectory(test)
