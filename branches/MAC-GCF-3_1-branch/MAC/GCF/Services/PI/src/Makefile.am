AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../MACCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
  GPI_Defines.h \
  GPI_Controller.h \
  GPI_SupervisoryServer.h \
  GPI_PropertyProxy.h \
  GPI_PropertySet.h
    
noinst_LTLIBRARIES 	= libpi.la

libpi_la_SOURCES= $(DOCHDRS) \
  GPI_Controller.cc \
  GPI_SupervisoryServer.cc \
  GPI_PropertyProxy.cc \
  GPI_PropertySet.cc \
  PI_Protocol.cc \
  $(top_builddir)/CoreComps/PA/src/PA_Protocol.cc
  
libpi_la_CPPFLAGS= -I$(top_srcdir)/GCFCommon/src \
	-I$(top_srcdir)/CoreComps/TM/src \
	-I$(top_srcdir)/CoreComps/SAL/src \
	-I$(top_srcdir)/CoreComps/PA/src \
	-I$(top_builddir)/CoreComps/PA/src

bin_PROGRAMS   		= PropertyInterface
  
PropertyInterface_SOURCES		= GPI_Main.cc
PropertyInterface_LDADD		      	= libpi.la  \
	$(top_builddir)/CoreComps/PML/src/libpml.la \
	$(top_builddir)/CoreComps/TM/src/libtm.la \
	$(top_builddir)/CoreComps/SAL/src/libsal.la \
	$(top_builddir)/GCFCommon/src/libgcfcommon.la
	
PropertyInterface_DEPENDENCIES	= libpi.la \
	$(top_builddir)/CoreComps/PML/src/libpml.la \
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
	
MAC_CONFIG = $(top_srcdir)/../Config
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.ns: $(MAC_CONFIG)/%.ns
	cp $< $@

%.top: $(MAC_CONFIG)/%.top
	cp $< $@

include $(lofar_sharedir)/Makefile.common

