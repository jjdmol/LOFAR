AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

DOCHDRS	=	 $(BUILT_SOURCES) \
	MISDaemon.h \
	MISDefines.h \
	MISSession.h \
	MISSubscription.h \
	PropertyProxy.h \
	MISPolicyHandler.h
   
noinst_LTLIBRARIES 	= libmis.la

libmis_la_SOURCES	= $(DOCHDRS) \
	MISDaemon.cc \
	MISSession.cc \
	MISSubscription.cc \
	PropertyProxy.cc \
	MISPolicyHandler.cc \
	MIS_Protocol.cc
	
bin_PROGRAMS   		= MACInformationServer
  
MACInformationServer_SOURCES		  = MISMain.cc
MACInformationServer_LDADD		    = libmis.la $(LOFAR_DEPEND)
MACInformationServer_DEPENDENCIES	= libmis.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	MIS_Protocol.ph

EXTRA_DIST = $(sysconf_DATA)
	
sysconf_DATA= \
	MACInformationServer.conf
	
%.conf: %.conf.in
	cp $< $@

clean-local:
	rm -f *.ph
	
include $(top_srcdir)/Makefile.common

