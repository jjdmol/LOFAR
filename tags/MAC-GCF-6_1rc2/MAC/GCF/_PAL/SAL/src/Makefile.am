DOCHDRS	=	\
	GSA_SCADAHandler.h \
	GSA_PvssApi.h \
	GSA_Resources.h \
	GSA_Service.h \
	GSA_WaitForAnswer.h \
	GSA_Defines.h \
	GSA_PortService.h \
	GSA_SysConnGuard.h
    
noinst_LTLIBRARIES 	= libsal.la

libsal_la_SOURCES= $(DOCHDRS) \
	GSA_SCADAHandler.cc \
	GSA_PvssApi.cc \
	GSA_Resources.cc \
	GSA_Service.cc \
	GSA_WaitForAnswer.cc \
	GSA_PortService.cc \
	GCF_PVSSPort.cc \
	GCF_PVSSInfo.cc \
	GCF_SysConnGuard.cc \
	GSA_SysConnGuard.cc

AM_CPPFLAGS = -DLOFARLOGGER_SUBPACKAGE=\"SAL\"
		
include $(top_srcdir)/Makefile.common

