AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../GCFCommon/autogen $<

DOCHDRS	=	\
	GPM_Controller.h \
	GCF_SupTask.h \
	GPM_Property.h \
	GPM_PropertySet.h \
	GPM_PropertyProxy.h \
	GCF_PropertyProxy.h \
	GPM_Service.h \
	GPM_Defines.h
    
lib_LTLIBRARIES 	= libpml.la

libpml_la_SOURCES= $(DOCHDRS) \
	GPM_Controller.cc \
	GCF_SupTask.cc \
	GPM_Property.cc \
	GPM_PropertySet.cc \
	GPM_PropertyProxy.cc \
	GCF_PropertyProxy.cc \
	GPM_Service.cc 

BUILT_SOURCES = \
	PA_Protocol.ph

EXTRA_DIST = \
	PA_Protocol.ph

include $(lofar_sharedir)/Makefile.common

