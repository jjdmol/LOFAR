#
# automake specification for TH_Corba transport.
#

#
# We need these lines to force automake
# to add CorbaTransportInterface.o to the OBJECTS
# list. The real .idl rules are in Makefile.common
#

include $(lofar_sharedir)/Makefile.common


# Definition of target library
lib_LTLIBRARIES 	= libth_corba.la


DOCHDRS			= 			\
BS_Corba.h					\
MTCircularBuffer.h				\
CorbaController.h				\
CorbaMonitor.h					\
CorbaTransport.h				\
CorbaTransportIn.h				\
CorbaTransportOut.h

# Need this variable to ensure that idl sources get compiled (first).
BUILT_SOURCES = 				\
CorbaTransportInterface.cs_cc			\
CorbaControl.cs_cc

libth_corba_la_SOURCES = \
$(DOCHDRS)					\
CorbaTransportInterface.idl			\
CorbaControl.idl				\
CorbaTransportInterface.cc			\
CorbaControl.cc				\
BS_Corba.cc					\
CorbaController.cc				\
CorbaMonitor.cc					\
CorbaTransport.cc				\
CorbaTransportIn.cc				\
CorbaTransportOut.cc				\
TH_Corba.cc

