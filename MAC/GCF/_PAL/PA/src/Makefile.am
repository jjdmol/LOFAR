AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../MACCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
	GPA_Controller.h \
	GPA_PropertySet.h \
	GPA_RequestManager.h \
	GPA_Converter.h \
	GPA_Defines.h

noinst_LTLIBRARIES 	= libpa.la

libpa_la_SOURCES= $(DOCHDRS) \
	GPA_Controller.cc \
	GPA_PropertySet.cc \
	GPA_RequestManager.cc \
	GPA_Converter.cc \
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
	preparePVSS-DB \
	configurePVSSProj \
	PropertyAgent.conf

install-data-local:
	mkdir -p $(bindir)/pvss/scripts
	mkdir -p $(bindir)/pvss/config
	cp -Rf $(top_srcdir)/PA/pvss/scripts $(bindir)/pvss/
	cp -Rf $(top_srcdir)/PA/pvss/config/config.level $(bindir)/pvss/config/
	chmod 777 $(bindir)/configurePVSSProj
	
%.conf: gcf-pa.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

