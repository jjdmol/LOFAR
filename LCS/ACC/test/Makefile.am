
bin_PROGRAMS			= tParameterSet tACClient ACuserMenu APExample APTest

### tParameterSet ###
tParameterSet_SOURCES 	   = tParameterSet.cc
tParameterSet_DEPENDENCIES = $(LOFAR_DEPEND) ../src/libacc.la
tParameterSet_LDADD		   = ../src/libacc.la

### tACClient ###
tACClient_SOURCES 		   = tACClient.cc 
tACClient_DEPENDENCIES	   = $(LOFAR_DEPEND) ../src/libacc.la
tACClient_LDADD			   = ../src/libacc.la

### ACuserMenu ###
ACuserMenu_SOURCES 		   = ACuserMenu.cc 
ACuserMenu_DEPENDENCIES	   = $(LOFAR_DEPEND) ../src/libacc.la
ACuserMenu_LDADD		   = ../src/libacc.la

### APExample ###
APExample_SOURCES 		   = APCmdImpl.cc APExample.cc 
APExample_DEPENDENCIES	   = $(LOFAR_DEPEND) ../src/libacc.la
APExample_LDADD		       = ../src/libacc.la

### APTest ###
APTest_SOURCES 		       = APCmdImpl.cc APTest.cc
APTest_DEPENDENCIES	       = $(LOFAR_DEPEND) ../src/libacc.la
APTest_LDADD			   = ../src/libacc.la

include $(lofar_sharedir)/Makefile.common

