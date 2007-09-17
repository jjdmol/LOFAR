check_PROGRAMS				= tACServer tACClient tACCmd

tACServer_SOURCES			= tACServer.cc 
tACClient_SOURCES			= tACClient.cc 
tACCmd_SOURCES				= tACCmd.cc 

noinst_HEADERS              = myACClientFunctions.h

LDADD						= ../src/libalc.la
DEPENDENCIES				= ../src/libalc.la $(LOFAR_DEPEND)

TESTSCRIPTS					=

TESTS						= $(TESTSCRIPTS)


EXTRA_DIST					= $(TESTSCRIPTS)

include $(top_srcdir)/Makefile.common

