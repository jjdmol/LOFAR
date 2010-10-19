pkginclude_HEADERS = \
	../../include/GCF/PAL/GCF_PVSSPort.h \
	../../include/GCF/PAL/GCF_PVSSInfo.h

DOCHDRS	=	$(pkginclude_HEADERS) \
	GSA_SCADAHandler.h \
	GSA_PvssApi.h \
	GSA_Resources.h \
	GSA_Service.h \
	GSA_WaitForAnswer.h \
	GSA_Defines.h \
	GSA_PortService.h
    
noinst_LTLIBRARIES 	= libsal.la

libsal_la_SOURCES= $(DOCHDRS) \
	GSA_SCADAHandler.cc \
	GSA_PvssApi.cc \
	GSA_Resources.cc \
	GSA_Service.cc \
	GSA_WaitForAnswer.cc \
	GSA_PortService.cc \
	GCF_PVSSPort.cc \
	GCF_PVSSInfo.cc

libsal_la_CPPFLAGS= -I$(top_srcdir)/include
		
include $(lofar_sharedir)/Makefile.common

