
check_PROGRAMS			   = ACuserMenu APExample APTest

TESTS					   = 

### ACuserMenu ###
ACuserMenu_SOURCES 		   = ACuserMenu.cc 
ACuserMenu_DEPENDENCIES	   = $(LOFAR_DEPEND) 
ACuserMenu_LDADD		   = 

### APExample ###
APExample_SOURCES 		   = APCmdImpl.cc APExample.cc 
APExample_DEPENDENCIES	   = $(LOFAR_DEPEND) 
APExample_LDADD		       = 

### APTest ###
APTest_SOURCES 		       = APCmdImpl.cc APTest.cc
APTest_DEPENDENCIES	       = $(LOFAR_DEPEND) 
APTest_LDADD			   = 

TESTS_ENVIRONMENT			= lofar_sharedir=$(lofar_sharedir) \
							  LOFARROOT=$(LOFARROOT)

include $(top_srcdir)/Makefile.common

