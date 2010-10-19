include_HEADERS = 

DOCHDRS	=	$(include_HEADERS) \
	GPM_Controller.h \
	GPM_PropertyService.h \
	GPM_Defines.h
    
noinst_LTLIBRARIES 	= libpml.la

libpml_la_SOURCES= $(DOCHDRS) \
	GCF_PropertyBase.cc \
	GCF_Property.cc \
	GCF_MyProperty.cc \
	GCF_PropertySetBase.cc \
	GCF_PropertySet.cc \
	GCF_MyPropertySet.cc \
	GCF_Apc.cc \
	GCF_Answer.cc \
	GPM_Controller.cc \
	GPM_PropertyService.cc \
	GCF_PropertyProxy.cc

libpml_la_CPPFLAGS= -I$(top_srcdir)/GCFCommon/src \
	-I$(top_srcdir)/CoreComps/TM/src \
	-I$(top_srcdir)/CoreComps/SAL/src \
	-I$(top_srcdir)/CoreComps/PA/src \
	-I$(top_builddir)/CoreComps/PA/src
	
include $(lofar_sharedir)/Makefile.common

