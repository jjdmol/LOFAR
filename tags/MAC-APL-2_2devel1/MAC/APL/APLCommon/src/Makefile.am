pkgincludedir = $(includedir)/APLCommon
pkginclude_HEADERS = \
	../include/APLCommon/APLInterTaskPort.h \
	../include/APLCommon/APL_Defines.h

DOCHDRS	=	$(include_HEADERS) \
	APLInterTaskPort.h
      
lib_LTLIBRARIES 	= libaplcommon.la

libaplcommon_la_CPPFLAGS= \
  -Wno-deprecated -I$(top_srcdir)/include

libaplcommon_la_SOURCES= $(DOCHDRS) \
	APLInterTaskPort.cc

include $(lofar_sharedir)/Makefile.common

