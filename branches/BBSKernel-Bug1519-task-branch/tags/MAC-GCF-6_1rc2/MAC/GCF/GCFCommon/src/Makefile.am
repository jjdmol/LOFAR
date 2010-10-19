DOCHDRS	=	GCO_Defines.h
      
lib_LTLIBRARIES 	= libgcfcommon.la

libgcfcommon_la_DEPENDENCIES	= $(LOFAR_DEPEND)

libgcfcommon_la_SOURCES= $(DOCHDRS) \
	CmdLine.cc \
	Utils.cc \
	GCF_PValue.cc \
	GCF_PVChar.cc \
	GCF_PVBool.cc \
	GCF_PVDouble.cc \
	GCF_PVString.cc \
	GCF_PVInteger.cc \
	GCF_PVUnsigned.cc \
	GCF_PVDynArr.cc \
	GCF_PVBlob.cc \
	ParameterSet.cc \
	Thread.cc
	
include $(top_srcdir)/Makefile.common

