# $Id$

lofar_add_package(CobaltIntegrationTests IntegrationTests)
lofar_package(Online_Cobalt DEPENDS GPUProc CobaltIntegrationTests APLCommon)

