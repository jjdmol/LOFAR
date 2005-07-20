INSTALL=/usr/bin/install -C

AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

DOCHDRS	=	 \
	MISDaemon.h \
	MISDefines.h \
	MISSession.h \
	MISSubscription.h \
	PropertyProxy.h \
	MIS_Protocol.ph
   
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
MACInformationServer_LDADD		    = libmis.la $(LOFAR_DEPEND)	-lboost_regex-gcc-1_32
MACInformationServer_DEPENDENCIES	= libmis.la $(LOFAR_DEPEND)
MACInformationServer_CPPFLAGS			=

BUILT_SOURCES = \
	MIS_Protocol.ph

EXTRA_DIST = \
	MIS_Protocol.ph
	
configfilesdir=$(bindir)
configfiles_DATA= \
	MIS.conf
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

