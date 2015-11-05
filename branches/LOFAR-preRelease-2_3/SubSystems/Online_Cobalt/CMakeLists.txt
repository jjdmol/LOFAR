# $Id$

# should cover everything that Online_OutputProc has (e.g. for tests on cbt009)
#
# APLCommon is needed for the `swlevel' script
lofar_package(Online_Cobalt DEPENDS Cobalt Online_OutputProc APLCommon)

add_subdirectory(test)
