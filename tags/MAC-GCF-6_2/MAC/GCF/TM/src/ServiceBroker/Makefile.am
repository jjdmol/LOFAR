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

EXTRA_DIST = $(sysconf_DATA)

sysconf_DATA= \
	ServiceBroker.conf

install-data-local:
	f=$(DESTDIR)$(sysconfdir)/ServiceBroker.conf; \
	hs=`hostname -s`; \
	h=`hostname`; \
	n=`sed -e  "/mac.gcf.sb.host=/ s/\(.*\)=\(.*\)/\1=$$hs/" -e "/mac.gcf.sb.range1.host=/ s/\(.*\)=\(.*\)/\1=$$h/" $$f`; \
	echo "$$n" > $$f;
	
%.conf: gcf-sb.conf.in
	cp $< $@
	
include $(top_srcdir)/Makefile.common

