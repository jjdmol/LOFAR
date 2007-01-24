AUTOGEN 							= autogen
SUFFIXES 							= .ph

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

DOCHDRS								= KVLDefines.h \
									  KeyValueLoggerDaemon.h \
									  KeyValueLoggerMaster.h \
									  KVLUtils.h \
									  $(BUILT_SOURCES)
#									  PropertyLogger.h
   
bin_PROGRAMS   						= KeyValueLoggerDaemon KeyValueLoggerMaster 
lib_LTLIBRARIES 					= libgcfkvlc.la libcepkvlc.la
noinst_LTLIBRARIES 					= libkvld.la libkvlm.la libkvlprot.la

libgcfkvlc_la_SOURCES				= $(DOCHDRS) GCF_KeyValueLogger.cc
libgcfkvlc_la_LIBADD				= libkvlprot.la
libgcfkvlc_la_DEPENDENCIES			= libkvlprot.la $(LOFAR_DEPEND)
libgcfkvlc_la_CPPFLAGS				= -DLOFARLOGGER_SUBPACKAGE=\"KVLC\"

libcepkvlc_la_SOURCES				= $(DOCHDRS) CEPKeyValueLogger.cc
libcepkvlc_la_LIBADD				= libkvlprot.la
libcepkvlc_la_DEPENDENCIES			= libkvlprot.la $(LOFAR_DEPEND)

libkvlprot_la_SOURCES				= $(DOCHDRS) \
									  KVL_Protocol.cc \
									  KVLUtils.cc
  
libkvld_la_SOURCES					= $(DOCHDRS) \
									  KeyValueLoggerDaemon.cc
#									  PropertyLogger.cc
libkvld_la_LIBADD					= libkvlprot.la
libkvld_la_CPPFLAGS					= -DLOFARLOGGER_SUBPACKAGE=\"KVLD\"

KeyValueLoggerDaemon_SOURCES		= KVLDMain.cc
KeyValueLoggerDaemon_LDADD		    = libkvld.la $(LOFAR_DEPEND)
KeyValueLoggerDaemon_DEPENDENCIES	= libkvld.la $(LOFAR_DEPEND)
KeyValueLoggerDaemon_CPPFLAGS		= $(libkvld_la_CPPFLAGS)

libkvlm_la_SOURCES					= $(DOCHDRS) \
									  KeyValueLoggerMaster.cc
libkvlm_la_LIBADD					= libkvlprot.la
libkvlm_la_CPPFLAGS					= -DLOFARLOGGER_SUBPACKAGE=\"KVLM\"

KeyValueLoggerMaster_SOURCES		= KVLMMain.cc
KeyValueLoggerMaster_LDADD		    = libkvlm.la $(LOFAR_DEPEND)
KeyValueLoggerMaster_DEPENDENCIES	= libkvlm.la $(LOFAR_DEPEND)
KeyValueLoggerMaster_CPPFLAGS		= $(libkvlm_la_CPPFLAGS)

BUILT_SOURCES 						= KVL_Protocol.ph \
									  KVL_Protocol.cc

EXTRA_DIST 							= $(sysconf_DATA)
	
sysconf_DATA						= KeyValueLoggerDaemon.conf \
									  KeyValueLoggerMaster.conf \
									  KeyValueLoggerDaemon.log_prop \
									  KeyValueLoggerMaster.log_prop
	

%.conf: KeyValueLoggerSys.conf.in
	cp $< $@

%.log_prop: KeyValueLoggerSys.log_prop.in
	cp $< $@
	
clean-local:
	rm -f *.ph
	
include $(top_srcdir)/Makefile.common

