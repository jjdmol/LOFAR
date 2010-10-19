AUTOGEN = autogen
SUFFIXES = .ph
%.ph: $(top_srcdir)/CoreComps/PA/src/%.prot
	$(AUTOGEN) -L $(top_srcdir)/../MACCommon/autogen $< && \
	chmod a+w $@

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
	GCF_PropertyProxy.cc \
	PA_Protocol.cc

libpml_la_CPPFLAGS= -I$(top_srcdir)/GCFCommon/src \
	-I$(top_srcdir)/CoreComps/TM/src \
	-I$(top_srcdir)/CoreComps/SAL/src \
	-I$(top_srcdir)/CoreComps/PA/src \
	-I$(top_builddir)/CoreComps/PA/src
	
BUILT_SOURCES = \
	PA_Protocol.ph
	
EXTRA_DIST = \
	PA_Protocol.ph

include $(lofar_sharedir)/Makefile.common

