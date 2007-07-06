NODOCHDRS			= GSA_SCADAHandler.h \
					  GSA_PvssApi.h \
					  GSA_Resources.h \
					  WaitForAnswer.h \
    
DOCHDRS				= PVSSservice.h \
					  PVSSresponse.h \
					  PVSSresult.h \
					  PVSSinfo.h

inst_LTLIBRARIES 	= libpvss.la

libpvss_la_SOURCES	= $(DOCHDRS) $(NODOCHDRS) \
					  GSA_SCADAHandler.cc \
					  GSA_PvssApi.cc \
					  GSA_Resources.cc \
					  WaitForAnswer.cc \
					  PVSSinfo.cc \
					  PVSSservice.cc \
					  PVSSresponse.cc

AM_CPPFLAGS 		= -DLOFARLOGGER_SUBPACKAGE=\"PVSS\"
		
include $(top_srcdir)/Makefile.common

