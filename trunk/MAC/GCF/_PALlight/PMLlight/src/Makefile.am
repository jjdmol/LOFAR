INSTALL=/usr/bin/install -C
pkgincludedir = $(includedir)/GCF/PALlight
pkginclude_HEADERS = \
	../../include/GCF/PALlight/GCF_RTMyProperty.h \
	../../include/GCF/PALlight/GCF_RTMyPropertySet.h \
	../../include/GCF/PALlight/GCF_RTAnswer.h
	
DOCHDRS	=	$(pkginclude_HEADERS) \
  GPM_RTController.h \
  GPM_RTDefines.h
        
noinst_LTLIBRARIES 	= libpmllight.la

libpmllight_la_SOURCES= $(DOCHDRS) \
  GCF_RTAnswer.cc \
  GCF_RTMyProperty.cc \
  GCF_RTMyPropertySet.cc \
  GPM_RTController.cc
  
libpmllight_la_CPPFLAGS= -I$(top_srcdir)/include

lib_LTLIBRARIES 	= libgcfpallight.la

libgcfpallight_la_SOURCES=
libgcfpallight_la_LIBADD= libpmllight.la

libgcfpallight_la_DEPENDENCIES= libpmllight.la $(LOFAR_DEPEND)

AM_CPPFLAGS = -DLOFARLOGGER_SUBPACKAGE=\"RTC\"

configfilesdir=$(bindir)
configfiles_DATA= \
	gcf-pmllight.conf \
	genTypeInfo

%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

