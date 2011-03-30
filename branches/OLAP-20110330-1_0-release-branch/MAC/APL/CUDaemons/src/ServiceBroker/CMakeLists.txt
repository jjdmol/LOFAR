# $Id$

# Add current directory to the -I path.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_bin_program(ServiceBroker ServiceBrokerMain.cc ServiceBroker.cc)

install(FILES ServiceBroker.conf DESTINATION etc)
