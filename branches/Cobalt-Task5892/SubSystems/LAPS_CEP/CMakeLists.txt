# $Id$

# Online_OutputProc is needed for tests on cbt nodes
# APLCommon is needed for the `swlevel' script
# PVSSGateway_Stub and Service_Broker are needed for the tRTmetadataToFile test
lofar_package(LAPS_CEP DEPENDS Offline LAPS)

add_subdirectory(test)
