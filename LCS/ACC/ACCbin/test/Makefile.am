
check_PROGRAMS		= APExample

bin_PROGRAMS		= APTest
sysconf_DATA		= APTest.log_prop

TESTS			= 

### APExample ###
APExample_SOURCES	= APCmdImpl.cc APExample.cc 
APExample_DEPENDENCIES	= $(LOFAR_DEPEND) APCmdImpl.h
APExample_LDADD		= 

### APTest ###
APTest_SOURCES		= APCmdImpl.cc APTest.cc
APTest_DEPENDENCIES	= $(LOFAR_DEPEND) APCmdImpl.h
APTest_LDADD		= 

include $(top_srcdir)/Makefile.common
