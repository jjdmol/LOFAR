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
					ApplControlComm.cc \
					ApplControlClient.cc \
					ACSyncClient.cc  \
					ACAsyncClient.cc \
					DH_ApplControl.cc \
					DH_AC_Connect.cc  \
					ApplControlServer.cc 
#					ParCollRecord.cc 
#					ConfigurationMgr.cc 

bin_PROGRAMS = ApplController

### ApplController ###
ApplController_SOURCES      = ACCmdImpl.cc \
							  DataHolderSelector.cc \
							  ACCommand.cc \
							  CmdStack.cc \
							  DH_ProcessControl.cc  \
							  ApplController.cc
# ApplProcess.cc 
# ApplProcessPool.cc 

ApplController_DEPENDENCIES = $(LOFAR_DEPEND) libacc.la
ApplController_LDADD        = libacc.la

include $(lofar_sharedir)/Makefile.common

