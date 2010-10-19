include_HEADERS = \
	GCF_PValue.h \
	GCF_PVBool.h \
	GCF_PVDouble.h \
	GCF_PVInteger.h \
	GCF_PVUnsigned.h \
	GCF_PVString.h \
	GCF_PVChar.h \
	GCF_PVDynArr.h 

DOCHDRS	=	$(include_HEADERS) \
	GSA_SCADAHandler.h \
	GSA_PvssApi.h \
	GSA_Resources.h \
	GSA_Service.h \
	GSA_WaitForAnswer.h \
	GSA_Defines.h
    
lib_LTLIBRARIES 	= libsal.la

libsal_la_SOURCES= $(DOCHDRS) \
	GSA_SCADAHandler.cc \
	GSA_PvssApi.cc \
	GSA_Resources.cc \
	GSA_Service.cc \
	GCF_PVBool.cc \
	GCF_PValue.cc \
	GCF_PVDouble.cc \
	GCF_PVInteger.cc \
	GCF_PVUnsigned.cc \
	GCF_PVString.cc \
	GCF_PVChar.cc \
	GCF_PVDynArr.cc \
	GSA_WaitForAnswer.cc

include $(lofar_sharedir)/Makefile.common

