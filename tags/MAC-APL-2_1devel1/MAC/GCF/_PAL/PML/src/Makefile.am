AUTOGEN = autogen
SUFFIXES = .ph
%.ph: $(top_srcdir)/PA/src/%.prot
	$(AUTOGEN) -L $(top_srcdir)/../../MACCommon/autogen $< && \
	chmod a+w $@

pkgincludedir = $(includedir)/GCF/PAL
pkginclude_HEADERS = \
	../../include/GCF/PAL/GCF_ExtProperty.h \
	../../include/GCF/PAL/GCF_Property.h \
	../../include/GCF/PAL/GCF_MyProperty.h \
	../../include/GCF/PAL/GCF_ExtPropertySet.h \
	../../include/GCF/PAL/GCF_PropertySet.h \
	../../include/GCF/PAL/GCF_MyPropertySet.h \
	../../include/GCF/PAL/GCF_Answer.h \
	../../include/GCF/PAL/GCF_PropertyProxy.h
	
DOCHDRS	=	$(pkginclude_HEADERS) \
	GPM_Controller.h \
	GPM_PropertyService.h \
	GPM_PropertyProxy.h \
	GPM_Defines.h
   
lib_LTLIBRARIES 	= libgcfpal.la

libgcfpal_la_SOURCES=
libgcfpal_la_LIBADD= libpml.la\
	$(top_builddir)/SAL/src/libsal.la
libgcfpal_la_LDFLAGS= -static
libgcfpal_la_DEPENDENCIES= libpml.la $(LOFAR_DEPEND) \
	$(top_builddir)/SAL/src/libsal.la


noinst_LTLIBRARIES 	= libpml.la

libpml_la_SOURCES= $(DOCHDRS) \
	GCF_ExtProperty.cc \
	GCF_Property.cc \
	GCF_MyProperty.cc \
	GCF_ExtPropertySet.cc \
	GCF_PropertySet.cc \
	GCF_MyPropertySet.cc \
	GCF_Answer.cc \
	GPM_Controller.cc \
	GPM_PropertyService.cc \
	GCF_PropertyProxy.cc \
	PA_Protocol.cc

libpml_la_CPPFLAGS= -I$(top_srcdir)/include \
	-I$(top_srcdir)/SAL/src \
	-I$(top_srcdir)/PA/src
libpml_la_LDFLAGS= -static
	
BUILT_SOURCES = \
	PA_Protocol.ph
	
EXTRA_DIST = \
	PA_Protocol.ph

configfilesdir=$(bindir)
configfiles_DATA= \
	gcf-pml.conf

%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

