DOCHDRS	=	\
	GSA_SCADAHandler.h \
	GSA_PvssApi.h \
	GSA_Resources.h \
	GSA_Service.h \
	GSA_WaitForAnswer.h \
	GCF_PValue.h \
	GCF_PVBool.h \
	GCF_PVDouble.h \
	GAS_Defines.h
    
lib_LTLIBRARIES 	= libsal.la

libsal_la_SOURCES= $(DOCHDRS) \
	GSA_SCADAHandler.cc \
	GSA_PvssApi.cc \
	GSA_Resources.cc \
	GSA_Service.cc \
	GCF_PVBool.cc \
	GCF_PValue.cc \
	GCF_PVDouble.cc \
	GSA_WaitForAnswer.cc

include $(lofar_sharedir)/Makefile.common

