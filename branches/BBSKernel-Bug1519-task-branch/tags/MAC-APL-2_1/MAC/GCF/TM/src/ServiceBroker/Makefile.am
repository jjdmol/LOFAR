DOCHDRS	=	\
	GSB_Controller.h \
	GSB_Defines.h

noinst_LTLIBRARIES 	= libsb.la

libsb_la_SOURCES= $(DOCHDRS) \
	GSB_Controller.cc
	
libsb_la_CPPFLAGS= -I$(top_srcdir)/include
libsb_la_CXXFLAGS=-fmessage-length=0

bin_PROGRAMS   		= ServiceBroker
  
ServiceBroker_SOURCES		    	= GSB_Main.cc
ServiceBroker_LDADD		     		= libsb.la $(LOFAR_DEPEND) \
	$(top_builddir)/src/libgcftm.la
	
ServiceBroker_DEPENDENCIES	= libsb.la $(LOFAR_DEPEND) \
	$(top_builddir)/src/libgcftm.la
	
ServiceBroker_CPPFLAGS= -I$(top_srcdir)/include
ServiceBroker_CXXFLAGS=-fmessage-length=0

BUILT_SOURCES = \
	log4cplus.properties \
	ServiceBroker.conf

EXTRA_DIST = \
	log4cplus.properties \
	ServiceBroker.conf

MAC_CONFIG = $(top_srcdir)/../../Config

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
configfilesdir=$(bindir)
configfiles_DATA= \
	ServiceBroker.conf \
	log4cplus.properties

install-data-local:
	echo "mac.gcf.sb.host=`hostname -s`" >> $(bindir)/ServiceBroker.conf
	echo "mac.gcf.sb.range1.host=`hostname`" >> $(bindir)/ServiceBroker.conf
	
%.conf: gcf-sb.conf.in
	cp $< $@
	
include $(lofar_sharedir)/Makefile.common

