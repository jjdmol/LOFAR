AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/GCFCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
	GPA_Controller.h \
	GPA_ScopeManager.h \
	GPA_UsecountManager.h \
	GPA_RequestManager.h \
	GPA_Defines.h \
	GPA_APCFileReader.h

noinst_LTLIBRARIES 	= libpa.la

libpa_la_SOURCES= $(DOCHDRS) \
	GPA_Controller.cc \
	GPA_ScopeManager.cc \
	GPA_UsecountManager.cc \
	GPA_RequestManager.cc \
	GPA_APCFileReader.cc
	
libpa_la_CPPFLAGS= -I$(top_srcdir)/GCFCommon/src \
	-I$(top_srcdir)/CoreComps/TM/src \
	-I$(top_srcdir)/CoreComps/SAL/src

bin_PROGRAMS   		= PropertyAgent
  
PropertyAgent_SOURCES		    	= GPA_Main.cc
PropertyAgent_LDADD		     			= libpa.la \
	$(top_builddir)/CoreComps/SAL/src/libsal.la \
	$(top_builddir)/CoreComps/TM/src/libtm.la \
	$(top_builddir)/GCFCommon/src/libgcfcommon.la
	
PropertyAgent_DEPENDENCIES	= libpa.la $(LOFAR_DEPEND) \
	$(top_builddir)/CoreComps/SAL/src/libsal.la \
	$(top_builddir)/CoreComps/TM/src/libtm.la \
	$(top_builddir)/GCFCommon/src/libgcfcommon.la
	
PropertyAgent_CPPFLAGS= -I$(top_srcdir)/GCFCommon/src \
	-I$(top_srcdir)/CoreComps/TM/src \
	-I$(top_srcdir)/CoreComps/SAL/src

BUILT_SOURCES = \
	PA_Protocol.ph \
	mac.ns \
	mac.top \
	log4cplus.properties

EXTRA_DIST = \
	PA_Protocol.ph \
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

