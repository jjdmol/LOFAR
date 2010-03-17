check_PROGRAMS				= tPCCmd tACCmain

tPCCmd_SOURCES				= tPCCmd.cc 

tACCmain_SOURCES			= tACCmain.cc APCmdImpl.cc

LDADD						= ../src/libplc.la
DEPENDENCIES				= ../src/libplc.la $(LOFAR_DEPEND)

TESTSCRIPTS					=

TESTS						= $(TESTSCRIPTS)


EXTRA_DIST					= $(TESTSCRIPTS)

include $(top_srcdir)/Makefile.common

