AUTOGEN = autogen
SUFFIXES = .ph
%.ph: $(top_srcdir)/Services/PI/src/%.prot
	$(AUTOGEN) -L $(top_srcdir)/../MACCommon/autogen $< && \
	chmod a+w $@

pkgincludedir = $(includedir)/GCF/PALlight
pkginclude_HEADERS = \
	../../include/GCF/PALlight/GCF_RTMyProperty.h \
	../../include/GCF/PALlight/GCF_RTMyPropertySet.h \
	../../include/GCF/PALlight/GCF_RTAnswer.h
	
DOCHDRS	=	$(pkginclude_HEADERS) \
  GPM_RTController.h \
  GPM_RTDefines.h
        
noinst_LTLIBRARIES 	= libpmllite.la

libpmllite_la_SOURCES= $(DOCHDRS) \
  GCF_RTAnswer.cc \
  GCF_RTMyProperty.cc \
  GCF_RTMyPropertySet.cc \
  GPM_RTController.cc \
  PI_Protocol.cc
  
libpmllite_la_CPPFLAGS= -I$(top_srcdir)/GCFCommon/src \
	-I$(top_srcdir)/CoreComps/TM/src \
	-I$(top_srcdir)/CoreComps/SAL/src \
	-I$(top_srcdir)/Services/PI/src \
	-I$(top_builddir)/Services/SupServer/src \
	-I$(top_srcdir)/Services/PMLlite/src/GCF


lib_LTLIBRARIES 	= libgcfpallight.la

libgcfpallight_la_SOURCES=
libgcfpallight_la_LIBADD= libpmllight.la

libgcfpallight_la_DEPENDENCIES= libpmllight.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	PI_Protocol.ph
	
EXTRA_DIST = \
	PI_Protocol.ph
	
include $(lofar_sharedir)/Makefile.common

