# $Id$

lofar_package(Service_Broker 1.0 DEPENDS Common GCFTM MACIO)
# Add current directory to the -I path.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(LofarPackageVersion)

lofar_add_library(servicebroker
  Package__Version.cc)

lofar_add_bin_program(ServiceBroker ServiceBrokerMain.cc ServiceBroker.cc)

install(FILES ServiceBroker.conf DESTINATION etc)
