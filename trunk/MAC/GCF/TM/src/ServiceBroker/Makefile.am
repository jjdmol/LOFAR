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
	mac.log_prop \
	ServiceBroker.conf

EXTRA_DIST = \
	mac.log_prop \
	ServiceBroker.conf

MAC_CONFIG = $(top_srcdir)/../../Config

%.log_prop: $(MAC_CONFIG)/%.log_prop
	cp $< $@
	
configfilesdir=$(bindir)
configfiles_DATA= \
	ServiceBroker.conf \
	mac.log_prop

install-data-local:
	echo "mac.gcf.sb.host=`hostname -s`" >> $(bindir)/ServiceBroker.conf
	echo "mac.gcf.sb.range1.host=`hostname`" >> $(bindir)/ServiceBroker.conf
	
%.conf: gcf-sb.conf.in
	cp $< $@
	
include $(lofar_sharedir)/Makefile.common

