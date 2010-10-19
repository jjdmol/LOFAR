DOCHDRS	=	\
	GPM_Controller.h \
	GPM_PropertyService.h \
	GPM_PropertyProxy.h \
	GPM_Defines.h
   
lib_LTLIBRARIES 	= libgcfpal.la

libgcfpal_la_SOURCES=
libgcfpal_la_LIBADD= libpml.la \
	$(top_builddir)/SAL/src/libsal.la
	
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
	GCF_PropertyProxy.cc

libpml_la_CPPFLAGS= -I$(top_srcdir)/SAL/src

AM_CPPFLAGS = -DLOFARLOGGER_SUBPACKAGE=\"PML\"
	
BUILT_SOURCES = 

EXTRA_DIST = $(BUILT_SOURCES) gcf-pml.conf.in

sysconf_DATA= \
	gcf-pml.conf

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common

