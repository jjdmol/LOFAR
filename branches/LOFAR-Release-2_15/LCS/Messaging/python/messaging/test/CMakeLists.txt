# $Id: CMakeLists.txt 1576 2015-09-29 15:22:28Z loose $

include(LofarCTest)

lofar_add_test(t_messages)
lofar_add_test(t_messagebus)
lofar_add_test(t_RPC)
lofar_add_test(t_service_message_handler)
