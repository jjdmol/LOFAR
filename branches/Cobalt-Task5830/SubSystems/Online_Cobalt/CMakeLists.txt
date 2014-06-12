# $Id$

# Online_OutputProc is needed for tests on cbt nodes
# APLCommon is needed for the `swlevel' script
# PVSSGateway_Stub and Service_Broker are needed for the tRTmetadataToFile test
lofar_package(Online_Cobalt DEPENDS Cobalt Online_OutputProc APLCommon PVSSGateway_Stub Service_Broker)

add_subdirectory(test)
