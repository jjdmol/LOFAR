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
	GCF_Property.cc \
	GCF_PropertyProxy.cc \
	GCF_MyProperty.cc \
	GCF_ExtProperty.cc \
	GCF_PropertySet.cc \
	GCF_MyPropertySet.cc \
	GCF_ExtPropertySet.cc \
	GCF_Answer.cc \
	GCF_PropertySetAnswer.cc \
	GPM_Controller.cc \
	GPM_PropertyService.cc 

libpml_la_CPPFLAGS= -I$(top_srcdir)/SAL/src

AM_CPPFLAGS = -DLOFARLOGGER_SUBPACKAGE=\"PML\"
	
BUILT_SOURCES = 

EXTRA_DIST = $(sysconf_DATA)

include $(top_srcdir)/Makefile.common

