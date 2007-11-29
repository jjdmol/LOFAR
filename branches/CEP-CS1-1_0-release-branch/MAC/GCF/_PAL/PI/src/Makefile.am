DOCHDRS	=	\
  GPI_Defines.h \
  GPI_Controller.h \
  GPI_PMLlightServer.h \
  GPI_PropertyProxy.h \
  GPI_PropertySet.h \
  GPI_CEPServer.h \
  GPI_RTCServer.h \
  GPI_TH_Port.h
    
noinst_LTLIBRARIES 	= libpi.la

libpi_la_SOURCES= $(DOCHDRS) \
  GPI_Controller.cc \
  GPI_PMLlightServer.cc \
  GPI_CEPServer.cc \
  GPI_RTCServer.cc \
  GPI_PropertyProxy.cc \
  GPI_PropertySet.cc \
  GPI_TH_Port.cc

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

AM_CPPFLAGS = -DLOFARLOGGER_SUBPACKAGE=\"PI\"

BUILT_SOURCES = 

EXTRA_DIST = $(BUILT_SOURCES)

include $(top_srcdir)/Makefile.common

