AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../GCFCommon/autogen $<

DOCHDRS	=	\
	GPA_Controller.h \
	GPA_ScopeManager.h \
	GPA_UsecountManager.h \
	GPA_RequestManager.h \
	GPA_Defines.h
    
lib_LTLIBRARIES 	= libpml.la

libpml_la_SOURCES= $(DOCHDRS) \
	GPA_Controller.cc \
	GPA_ScopeManager.cc \
	GPA_UsecountManager.cc \
	GPA_RequestManager.cc \
	GPA_Main.cc 

BUILT_SOURCES = \
	PA_Protocol.ph

EXTRA_DIST = \
	PA_Protocol.ph

include $(lofar_sharedir)/Makefile.common

