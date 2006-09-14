check_PROGRAMS				= tACServer tACClient

tACServer_SOURCES			= tACServer.cc 
tACClient_SOURCES			= tACClient.cc 

noinst_HEADERS              = myACClientFunctions.h

LDADD						= ../src/libalc.la
DEPENDENCIES				= ../src/libalc.la $(LOFAR_DEPEND)

TESTSCRIPTS					=

TESTS						= $(TESTSCRIPTS)


EXTRA_DIST					= $(TESTSCRIPTS)

include $(top_srcdir)/Makefile.common

