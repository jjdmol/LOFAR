NODOCHDRS			= GSA_SCADAHandler.h \
					  GSA_PvssApi.h \
					  GSA_Resources.h \
					  GSA_WaitForAnswer.h 
    
lib_LTLIBRARIES 	= libgcfpvss.la

libgcfpvss_la_SOURCES=$(NODOCHDRS) \
					  GCF_PValue.cc \
					  GCF_PVChar.cc \
					  GCF_PVBool.cc \
					  GCF_PVDouble.cc \
					  GCF_PVString.cc \
					  GCF_PVInteger.cc \
					  GCF_PVUnsigned.cc \
					  GCF_PVDynArr.cc \
					  GCF_PVBlob.cc \
					  GSA_SCADAHandler.cc \
					  GSA_PvssApi.cc \
					  GSA_Resources.cc \
					  GSA_WaitForAnswer.cc \
					  PVSSinfo.cc \
					  PVSSservice.cc

AM_CPPFLAGS 		= -DLOFARLOGGER_SUBPACKAGE=\"PVSS\"
		
include $(top_srcdir)/Makefile.common

