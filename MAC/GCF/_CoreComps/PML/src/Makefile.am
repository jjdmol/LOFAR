include_HEADERS = \
	GCF_SupTask.h \
	GCF_PropertyProxy.h 

DOCHDRS	=	$(include_HEADERS) \
	GPM_Controller.h \
	GPM_Property.h \
	GPM_PropertySet.h \
	GPM_PropertyProxy.h \
	GPM_Service.h \
	GPM_Defines.h
    
lib_LTLIBRARIES 	= libgcf.la libpml.la

libpml_la_SOURCES= $(DOCHDRS) \
	GPM_Controller.cc \
	GCF_SupTask.cc \
	GPM_Property.cc \
	GPM_PropertySet.cc \
	GPM_PropertyProxy.cc \
	GCF_PropertyProxy.cc \
	GPM_Service.cc 

libgcf_la_SOURCES = 
libgcf_la_LIBADD= libpml.la \
	../../../../TM/build/gnu_debug/src/libtm.la \
	../../../../SAL/build/gnu_debug/src/libsal.la \
	../../../../../GCFCommon/build/gnu_debug/src/libgcfcommon.la
	
include $(lofar_sharedir)/Makefile.common

