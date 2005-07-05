INSTALL=/usr/bin/install -C

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
  PIClient.cc
  
libceppmllight_la_CPPFLAGS= -I$(top_srcdir)/include

lib_LTLIBRARIES 	= libgcfceppallight.la

libgcfceppallight_la_SOURCES=
libgcfceppallight_la_LIBADD= libceppmllight.la

libgcfceppallight_la_DEPENDENCIES= libceppmllight.la $(LOFAR_DEPEND)

AM_CPPFLAGS = -DLOFARLOGGER_SUBPACKAGE=\"CEP\"

include $(lofar_sharedir)/Makefile.common

