check_PROGRAMS				= tACServer tACClient

TESTS						= 

### tACserver ###
tACServer_SOURCES			= tACServer.cc 
tACServer_DEPENDENCIES		= $(LOFAR_DEPEND) ../src/libalc.la
tACServer_LDADD				= ../src/libalc.la

### tACClient ###
tACClient_SOURCES			= tACClient.cc 
tACClient_DEPENDENCIES		= $(LOFAR_DEPEND) \
							  myACClientFunctions.h ../src/libalc.la
tACClient_LDADD				= ../src/libalc.la

TESTS_ENVIRONMENT			= lofar_sharedir=$(lofar_sharedir) \
							  LOFARROOT=$(LOFARROOT)

include $(lofar_sharedir)/Makefile.common

