pkgincludedir = $(includedir)/GCF
pkginclude_HEADERS = \
	../include/GCF/GCF_PValue.h \
	../include/GCF/GCF_PVChar.h \
	../include/GCF/GCF_PVBool.h \
	../include/GCF/GCF_PVDouble.h \
	../include/GCF/GCF_PVString.h \
	../include/GCF/GCF_PVInteger.h \
	../include/GCF/GCF_PVUnsigned.h \
	../include/GCF/GCF_PVDynArr.h \
	../include/GCF/CmdLine.h \
	../include/GCF/Utils.h \
	../include/GCF/GCF_Defines.h

DOCHDRS	=	$(pkginclude_HEADERS)
      
lib_LTLIBRARIES 	= libgcfcommon.la

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
	GCF_PVDynArr.cc
	
libgcfcommon_la_CPPFLAGS=-I$(top_srcdir)/include

include $(lofar_sharedir)/Makefile.common

