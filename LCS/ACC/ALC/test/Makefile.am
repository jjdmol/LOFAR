check_PROGRAMS				= tACServer tACClient

tACServer_SOURCES			= tACServer.cc 
tACClient_SOURCES			= tACClient.cc 

LDADD						= ../src/libalc.la
DEPENDENCIES				= ../src/libalc.la $(LOFAR_DEPEND)

TESTSCRIPTS					=

TESTS						= $(TESTSCRIPTS)


TESTS_ENVIRONMENT			= lofar_sharedir=$(lofar_sharedir) \
							  LOFARROOT=$(LOFARROOT)

EXTRA_DIST					= \
							  $(TESTSCRIPTS)

include $(top_srcdir)/Makefile.common

