AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../MACCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
	GPA_Controller.h \
	GPA_PropertySet.h \
	GPA_RequestManager.h \
	GPA_Defines.h

noinst_LTLIBRARIES 	= libpa.la

libpa_la_SOURCES= $(DOCHDRS) \
	GPA_Controller.cc \
	GPA_PropertySet.cc \
	GPA_RequestManager.cc \
	PA_Protocol.cc
	
libpa_la_CPPFLAGS= -I$(top_srcdir)/include \
	-I$(top_srcdir)/SAL/src
libpa_la_LDFLAGS= -static

bin_PROGRAMS   		= PropertyAgent
  
PropertyAgent_SOURCES		    	= GPA_Main.cc
PropertyAgent_LDADD		     			= libpa.la $(LOFAR_DEPEND) \
	$(top_builddir)/SAL/src/libsal.la
	
PropertyAgent_DEPENDENCIES	= libpa.la $(LOFAR_DEPEND) \
	$(top_builddir)/SAL/src/libsal.la
	
PropertyAgent_CPPFLAGS= -I$(top_srcdir)/include \
	-I$(top_srcdir)/SAL/src

BUILT_SOURCES = \
	PA_Protocol.ph

EXTRA_DIST = \
	PA_Protocol.ph

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA= \
	loadAPC \
	PropertyAgent.conf

MAC_CONFIG = $(top_srcdir)/../../Config

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.conf: gcf-pa.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

