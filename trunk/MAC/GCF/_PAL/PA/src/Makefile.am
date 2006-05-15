DOCHDRS	=	\
	GPA_Controller.h \
	GPA_PropertySet.h \
	GPA_RequestManager.h \
	GPA_Converter.h \
	GPA_Defines.h \
	GPA_PropSetSession.h

noinst_LTLIBRARIES 	= libpa.la

libpa_la_SOURCES= $(DOCHDRS) \
	GPA_Controller.cc \
	GPA_PropertySet.cc \
	GPA_RequestManager.cc \
	GPA_Converter.cc \
	GPA_PropSetSession.cc
	

bin_PROGRAMS   		= PropertyAgent
  
PropertyAgent_SOURCES		    	= GPA_Main.cc
PropertyAgent_LDADD		     			= libpa.la $(LOFAR_DEPEND) \
	$(top_builddir)/SAL/src/libsal.la
	
PropertyAgent_DEPENDENCIES	= libpa.la $(LOFAR_DEPEND) \
	$(top_builddir)/SAL/src/libsal.la
	
AM_CPPFLAGS = -DLOFARLOGGER_SUBPACKAGE=\"PA\" -I$(top_srcdir)/SAL/src

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA= \
	loadAPC \
	preparePVSS-DB

BUILT_SOURCES = 

EXTRA_DIST = $(configfiles_DATA) $(sysconf_DATA) 

sysconf_DATA= \
	PropertyAgent.conf

%.conf: gcf-pa.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common

