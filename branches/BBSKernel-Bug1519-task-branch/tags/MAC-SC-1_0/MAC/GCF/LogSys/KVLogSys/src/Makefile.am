INSTALL=/usr/bin/install -C

AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../MACCommon/autogen $<

pkgincludedir = $(includedir)/GCF/LogSys
pkginclude_HEADERS = \
	../../include/GCF/LogSys/GCF_KeyValueLogger.h
	
DOCHDRS	=	$(pkginclude_HEADERS) \
	KVDefines.h \
	KeyValueLoggerDaemon.h \
	KeyValueLoggerMaster.h \
	KVLUtils.h \
	KVL_Protocol.ph
   
lib_LTLIBRARIES 	= libgcfkvlc.la

libgcfkvlc_la_SOURCES= $(DOCHDRS) \
	GCF_KeyValueLogger.cc
	
libgcfkvlc_la_LIBADD= libkvlprot.la
libgcfkvlc_la_DEPENDENCIES= libkvlprot.la $(LOFAR_DEPEND)
libgcfkvlc_la_CPPFLAGS= -I$(top_srcdir)/include \
	-DLOFARLOGGER_SUBPACKAGE=\"KVLC\"

noinst_LTLIBRARIES 	= libkvlprot.la libkvld.la libkvlm.la

libkvlprot_la_SOURCES= $(DOCHDRS) \
  KVL_Protocol.cc \
  KVLUtils.cc

libkvld_la_SOURCES	= $(DOCHDRS) \
	KeyValueLoggerDaemon.cc

libkvld_la_LIBADD		= libkvlprot.la
libkvld_la_CPPFLAGS= -DLOFARLOGGER_SUBPACKAGE=\"KVLD\"

libkvlm_la_SOURCES	= $(DOCHDRS) \
	KeyValueLoggerMaster.cc

libkvlm_la_LIBADD		= libkvlprot.la
libkvlm_la_CPPFLAGS= -DLOFARLOGGER_SUBPACKAGE=\"KVLM\"

bin_PROGRAMS   		= KeyValueLoggerDaemon KeyValueLoggerMaster
  
KeyValueLoggerDaemon_SOURCES		  = KVLDMain.cc
KeyValueLoggerDaemon_LDADD		    = libkvld.la $(LOFAR_DEPEND)
KeyValueLoggerDaemon_DEPENDENCIES	= libkvld.la $(LOFAR_DEPEND)
KeyValueLoggerDaemon_CPPFLAGS			= -DLOFARLOGGER_SUBPACKAGE=\"KVLD\"

KeyValueLoggerMaster_SOURCES		  = KVLMMain.cc
KeyValueLoggerMaster_LDADD		    = libkvlm.la $(LOFAR_DEPEND)
KeyValueLoggerMaster_DEPENDENCIES	= libkvlm.la $(LOFAR_DEPEND)
KeyValueLoggerMaster_CPPFLAGS			= -DLOFARLOGGER_SUBPACKAGE=\"KVLM\"

BUILT_SOURCES = \
	KVL_Protocol.ph

EXTRA_DIST = \
	KVL_Protocol.ph
	
configfilesdir=$(bindir)
configfiles_DATA= \
	KeyValueLoggerDaemon.conf \
	KeyValueLoggerMaster.conf \
	KeyValueLoggerDaemon.log_prop \
	KeyValueLoggerMaster.log_prop
	

%.conf: KeyValueLoggerSys.conf.in
	cp $< $@

%.log_prop: KeyValueLoggerSys.log_prop.in
	cp $< $@
	
include $(lofar_sharedir)/Makefile.common

