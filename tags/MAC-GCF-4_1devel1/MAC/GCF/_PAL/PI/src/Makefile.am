AUTOGEN = autogen
SUFFIXES = .ph

PA_Protocol.ph: $(top_srcdir)/PA/src/PA_Protocol.prot
	$(AUTOGEN) -L $(top_srcdir)/../../MACCommon/autogen $< && \
	chmod a+w $@

PI_Protocol.ph: PI_Protocol.prot
	$(AUTOGEN) -L $(top_srcdir)/../../MACCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
  GPI_Defines.h \
  GPI_Controller.h \
  GPI_PMLlightServer.h \
  GPI_PropertyProxy.h \
  GPI_PropertySet.h
    
noinst_LTLIBRARIES 	= libpi.la

libpi_la_SOURCES= $(DOCHDRS) \
  GPI_Controller.cc \
  GPI_PMLlightServer.cc \
  GPI_PropertyProxy.cc \
  GPI_PropertySet.cc \
  PI_Protocol.cc \
  PA_Protocol.cc

libpi_la_CPPFLAGS= -I$(top_srcdir)/include \
	-I$(top_srcdir)/PA/src
	
bin_PROGRAMS   		= PropertyInterface
  
PropertyInterface_SOURCES		= GPI_Main.cc
PropertyInterface_LDADD		      	= libpi.la  \
	$(top_builddir)/PML/src/libpml.la \
	$(top_builddir)/SAL/src/libsal.la \
	$(LOFAR_DEPEND)
	
PropertyInterface_DEPENDENCIES	= libpi.la \
	$(top_builddir)/PML/src/libpml.la \
	$(top_builddir)/SAL/src/libsal.la \
	$(LOFAR_DEPEND) 

BUILT_SOURCES = \
	PI_Protocol.ph \
	PA_Protocol.ph

EXTRA_DIST = \
	PI_Protocol.ph \
	PA_Protocol.ph

configfilesdir=$(bindir)
configfiles_DATA= \
	PropertyInterface.conf
		
%.conf: gcf-pi.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

