DOCHDRS	=	GCO_Defines.h
      
lib_LTLIBRARIES 	= libgcfcommon.la

libgcfcommon_la_DEPENDENCIES	= $(LOFAR_DEPEND)

libgcfcommon_la_SOURCES= $(DOCHDRS) \
	CmdLine.cc \
	Utils.cc \
	Thread.cc
	
include $(top_srcdir)/Makefile.common

