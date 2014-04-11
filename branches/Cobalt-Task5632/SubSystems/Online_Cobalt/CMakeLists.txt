# $Id$

# should cover everything that Online_OutputProc has (e.g. for tests on cbt009)
#
# APLCommon is needed for the `swlevel' script
# GPUProc is in Cobalt, but is apparently needed to link test/tMultiPartTABOutput
lofar_package(Online_Cobalt DEPENDS Cobalt GPUProc Online_OutputProc APLCommon)

add_subdirectory(test)
