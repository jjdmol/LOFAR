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
					ParameterCollection.cc \
					ParameterTemplate.cc \
					TemplateUnion.cc \
					ParameterUnion.cc \
					ParameterSet.cc \
					DH_ApplControl.cc \
					DH_AC_Connect.cc  \
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

bin_PROGRAMS = ApplController

### ApplController ###
ApplController_SOURCES      = ACCmdImpl.cc \
							  ACCommand.cc \
							  CmdStack.cc \
							  StateEngine.cc \
							  APAdmin.cc \
							  APAdminPool.cc \
							  ApplController.cc \
							  ApplControllerMain.cc
# ApplProcess.cc 
# ApplProcessPool.cc 
# DataHolderSelector.cc 
# CmdTimer.cc

ApplController_DEPENDENCIES = $(LOFAR_DEPEND) libacc.la
ApplController_LDADD        = libacc.la

include $(lofar_sharedir)/Makefile.common

