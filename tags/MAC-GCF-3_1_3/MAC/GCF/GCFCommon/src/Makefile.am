pkginclude_HEADERS = \
	CmdLine.h \
	Utils.h

DOCHDRS	=	$(pkginclude_HEADERS)
      
noinst_LTLIBRARIES 	= libgcfcommon.la

libgcfcommon_la_SOURCES= $(DOCHDRS) \
	CmdLine.cc \
	Utils.cc

include $(lofar_sharedir)/Makefile.common

