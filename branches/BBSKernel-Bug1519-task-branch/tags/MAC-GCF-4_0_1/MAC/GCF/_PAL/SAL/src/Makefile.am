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
	GSA_WaitForAnswer.cc
		
include $(lofar_sharedir)/Makefile.common

