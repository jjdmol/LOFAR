AUTOGEN = autogen
SUFFIXES = .ph
%.ph: $(top_srcdir)/Services/PI/src/%.prot
	$(AUTOGEN) -L $(top_srcdir)/GCFCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
  GSS_Defines.h \
  GSS_Controller.h
    
noinst_LTLIBRARIES 	= libsupserver.la

libsupserver_la_SOURCES= $(DOCHDRS) \
  GSS_Controller.cc
  
libsupserver_la_CPPFLAGS= -I$(top_srcdir)/GCFCommon/src \
	-I$(top_srcdir)/CoreComps/TM/src \
	-I$(top_srcdir)/CoreComps/SAL/src \
	-I$(top_srcdir)/Services/PI/src

bin_PROGRAMS   		= SupervisoryServer
  
SupervisoryServer_SOURCES		= GSS_Main.cc
SupervisoryServer_LDADD		      	= libsupserver.la  \
	$(top_builddir)/CoreComps/TM/src/libtm.la \
	$(top_builddir)/CoreComps/SAL/src/libsal.la \
	$(top_builddir)/GCFCommon/src/libgcfcommon.la
	
SupervisoryServer_DEPENDENCIES	= libsupserver.la \
	$(top_builddir)/GCFCommon/src/libgcfcommon.la \
	$(top_builddir)/CoreComps/TM/src/libtm.la \
	$(top_builddir)/CoreComps/SAL/src/libsal.la \
	$(LOFAR_DEPEND) 

BUILT_SOURCES = \
	PI_Protocol.ph \
	mac.ns \
	mac.top \
	log4cplus.properties

EXTRA_DIST = \
	PI_Protocol.ph \
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

