DOCHDRS	=	\
  PIClient.h \
  PICEPDefines.h
        
noinst_LTLIBRARIES 	= libceppmllight.la

libceppmllight_la_SOURCES= $(DOCHDRS) \
  CEPProperty.cc \
  CEPPropertySet.cc \
  PIClient.cc
  
lib_LTLIBRARIES 	= libgcfceppallight.la

libgcfceppallight_la_SOURCES=
libgcfceppallight_la_LIBADD= libceppmllight.la

libgcfceppallight_la_DEPENDENCIES= libceppmllight.la $(LOFAR_DEPEND)

AM_CPPFLAGS = -DLOFARLOGGER_SUBPACKAGE=\"CEP\"

include $(top_srcdir)/Makefile.common

