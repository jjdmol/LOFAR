INSTHDRS = \
	ParameterCollection.h \
	ParameterTemplate.h \
	ParameterUnion.h \
	TemplateUnion.h \
	ParameterSet.h 

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES                 = libacc.la

libacc_la_SOURCES = \
					KVpair.cc \
					ParameterCollection.cc \
					ParameterTemplate.cc \
					TemplateUnion.cc \
					ParameterUnion.cc \
					ParameterSet.cc \
					DH_ApplControl.cc \
					ApplControlComm.cc \
					ApplControlClient.cc \
					ACSyncClient.cc  \
					ACAsyncClient.cc \
					ApplControlServer.cc \
					DH_ProcControl.cc \
					ProcControlComm.cc  \
					ProcControlServer.cc  
#					ParCollRecord.cc  
#					ConfigurationMgr.cc 

bin_PROGRAMS = ApplController ACDaemon

### ApplController ###
ApplController_SOURCES      = ACCmdImpl.cc \
							  CmdStack.cc \
							  StateEngine.cc \
							  APAdmin.cc \
							  APAdminPool.cc \
							  ProcRule.cc \
							  ProcRuler.cc \
							  ItemList.cc \
							  ApplController.cc \
							  ApplControllerMain.cc
ApplController_DEPENDENCIES = $(LOFAR_DEPEND) libacc.la
ApplController_LDADD        = libacc.la

### ACDaemon ###
ACDaemon_SOURCES      = ACRequestPool.cc \
						ACDaemon.cc \
					    ACDaemonMain.cc
ACDaemon_DEPENDENCIES = $(LOFAR_DEPEND) libacc.la
ACDaemon_LDADD        = libacc.la

include $(lofar_sharedir)/Makefile.common

