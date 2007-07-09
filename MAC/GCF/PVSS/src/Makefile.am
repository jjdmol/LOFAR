NODOCHDRS			= GSA_SCADAHandler.h \
					  GSA_PvssApi.h \
					  GSA_Resources.h \
					  GSA_WaitForAnswer.h 
    
lib_LTLIBRARIES 	= libpvss.la

libpvss_la_SOURCES	= $(NODOCHDRS) \
					  GSA_SCADAHandler.cc \
					  GSA_PvssApi.cc \
					  GSA_Resources.cc \
					  GSA_WaitForAnswer.cc \
					  PVSSinfo.cc \
					  PVSSservice.cc \
					  PVSSresponse.cc 

AM_CPPFLAGS 		= -DLOFARLOGGER_SUBPACKAGE=\"PVSS\"
		
include $(top_srcdir)/Makefile.common

