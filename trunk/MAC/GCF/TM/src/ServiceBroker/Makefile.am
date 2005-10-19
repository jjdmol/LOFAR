DOCHDRS	=	\
	GSB_Controller.h \
	GSB_Defines.h

noinst_LTLIBRARIES 	= libsb.la

libsb_la_SOURCES= $(DOCHDRS) \
	GSB_Controller.cc
	
libsb_la_CXXFLAGS=-fmessage-length=0

bin_PROGRAMS   		= ServiceBroker
  
ServiceBroker_SOURCES		    	= GSB_Main.cc
ServiceBroker_LDADD		     		= libsb.la $(LOFAR_DEPEND) \
	$(top_builddir)/src/libgcftm.la
	
ServiceBroker_DEPENDENCIES	= libsb.la $(LOFAR_DEPEND) \
	$(top_builddir)/src/libgcftm.la
	
ServiceBroker_CXXFLAGS=-fmessage-length=0

BUILT_SOURCES = 

EXTRA_DIST = $(BUILT_SOURCES) gcf-sb.conf.in

sysconf_DATA= \
	ServiceBroker.conf

install-data-local:
	echo "mac.gcf.sb.host=`hostname -s`" >> $(DESTDIR)$(sysconfdir)/ServiceBroker.conf
	echo "mac.gcf.sb.range1.host=`hostname`" >> $(DESTDIR)$(sysconfdir)/ServiceBroker.conf
	
%.conf: gcf-sb.conf.in
	cp $< $@
	
include $(top_srcdir)/Makefile.common

