
bin_PROGRAMS			= tParameterSet 
#						  tACClient tACMaster tACServer

### tParameterSet ###
tParameterSet_SOURCES 		= tParameterSet.cc
tParameterSet_DEPENDENCIES	= $(LOFAR_DEPEND)
tParameterSet_LDADD			= ../src/libacc.la

### tACClient ###
#tACClient_SOURCES 		= tACClient.cc 
#tACClient_DEPENDENCIES	= $(LOFAR_DEPEND) libacc.la
#tACClient_LDADD			= libacc.la

### tACMaster ###
#tACMaster_SOURCES 		= tACMaster.cc 
#tACMaster_DEPENDENCIES	= $(LOFAR_DEPEND) libacc.la
#tACMaster_LDADD			= libacc.la

### tACServer ###
#tACServer_SOURCES 		= tACServer.cc
#tACServer_DEPENDENCIES	= $(LOFAR_DEPEND) libacc.la
#tACServer_LDADD			= libacc.la

include $(lofar_sharedir)/Makefile.common

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
