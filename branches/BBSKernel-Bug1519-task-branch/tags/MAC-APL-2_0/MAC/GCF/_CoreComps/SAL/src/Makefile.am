pkginclude_HEADERS = 

DOCHDRS	=	$(pkginclude_HEADERS) \
	GSA_SCADAHandler.h \
	GSA_PvssApi.h \
	GSA_Resources.h \
	GSA_Service.h \
	GSA_WaitForAnswer.h \
	GSA_Defines.h
    
noinst_LTLIBRARIES 	= libsal.la

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
	
libsal_sa_CPPFLAGS=-I$(top_srcdir)/GCFCommon/src \
	-I$(top_srcdir)/CoreComps/TM/src
	
include $(lofar_sharedir)/Makefile.common

