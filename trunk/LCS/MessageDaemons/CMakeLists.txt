# $Id$

lofar_package(MessageDaemons 1.0 DEPENDS MessageBus)

include(LofarFindPackage)
lofar_find_package(QPID REQUIRED) # MessageBus may be able to fake it without QPID, but running the daemons without QPID is ridiculous.

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(webmonitor)

lofar_add_package(ObservationStartListener)
