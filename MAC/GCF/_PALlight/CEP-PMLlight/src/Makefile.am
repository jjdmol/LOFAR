INSTALL=/usr/bin/install -C
AUTOGEN = autogen
SUFFIXES = .ph

pkgincludedir = $(includedir)/GCF/PALlight
pkginclude_HEADERS = \
	../../include/GCF/PALlight/CEPProperty.h \
	../../include/GCF/PALlight/CEPPropertySet.h
	
DOCHDRS	=	$(pkginclude_HEADERS) \
  PIClient.h \
  PICEPDefines.h
        
noinst_LTLIBRARIES 	= libceppmllight.la

libceppmllight_la_SOURCES= $(DOCHDRS) \
  CEPProperty.cc \
  CEPPropertySet.cc \
  PIClient.cc \
  $(top_srcdir)/../PAL/PI/src/DH_PIProtocol.cc
  
libceppmllight_la_CPPFLAGS= -I$(top_srcdir)/include \
	-I$(top_srcdir)/../PAL/PI/src


lib_LTLIBRARIES 	= libgcfceppallight.la

libgcfceppallight_la_SOURCES=
libgcfceppallight_la_LIBADD= libceppmllight.la

libgcfceppallight_la_DEPENDENCIES= libceppmllight.la $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common

