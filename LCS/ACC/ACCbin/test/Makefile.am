
bin_PROGRAMS			= tParameterSet tACClient tPCClient ACuserMenu APExample

### tParameterSet ###
tParameterSet_SOURCES 		= tParameterSet.cc
tParameterSet_DEPENDENCIES	= $(LOFAR_DEPEND) ../src/libacc.la
tParameterSet_LDADD			= ../src/libacc.la

### tACClient ###
tACClient_SOURCES 		= tACClient.cc 
tACClient_DEPENDENCIES	= $(LOFAR_DEPEND) ../src/libacc.la
tACClient_LDADD			= ../src/libacc.la

### tPCClient ###
tPCClient_SOURCES 		= tPCClient.cc
tPCClient_DEPENDENCIES	= $(LOFAR_DEPEND) ../src/libacc.la
tPCClient_LDADD			= ../src/libacc.la

### ACuserMenu ###
ACuserMenu_SOURCES 		= ACuserMenu.cc 
ACuserMenu_DEPENDENCIES	= $(LOFAR_DEPEND) ../src/libacc.la
ACuserMenu_LDADD		= ../src/libacc.la

### APExample ###
APExample_SOURCES 		= APCmdImpl.cc APExample.cc 
APExample_DEPENDENCIES	= $(LOFAR_DEPEND) ../src/libacc.la
APExample_LDADD		= ../src/libacc.la

include $(lofar_sharedir)/Makefile.common

