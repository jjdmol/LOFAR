AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../GCFCommon/autogen $<

DOCHDRS	=	\
	GPA_Controller.h \
	GPA_ScopeManager.h \
	GPA_UsecountManager.h \
	GPA_RequestManager.h \
	GPA_Defines.h \
	GPA_APC.h

lib_LTLIBRARIES 	= libpa.la

libpa_la_SOURCES= $(DOCHDRS) \
	GPA_Controller.cc \
	GPA_ScopeManager.cc \
	GPA_UsecountManager.cc \
	GPA_RequestManager.cc \
	GPA_APC.cc

bin_PROGRAMS   		= PropertyAgent
  
PropertyAgent_SOURCES		    = GPA_Main.cc
PropertyAgent_LDADD		      = libpa.la 
PropertyAgent_DEPENDENCIES	= libpa.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	PA_Protocol.ph

EXTRA_DIST = \
	PA_Protocol.ph \
	mac.ns \
	mac.top \
	log4cplus.properties
	
pkgdata_DATA = \
	mac.ns \
	mac.top \
	log4cplus.properties

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.ns: $(MAC_CONFIG)/%.ns
	cp $< $@

%.top: $(MAC_CONFIG)/%.top
	cp $< $@

include $(lofar_sharedir)/Makefile.common

