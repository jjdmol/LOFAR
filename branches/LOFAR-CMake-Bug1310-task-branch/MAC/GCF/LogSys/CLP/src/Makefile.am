DOCHDRS								= CodeLoggingProcessor.h \
									  PropertyProxy.h
   
noinst_LTLIBRARIES 					= libclp.la

libclp_la_SOURCES					= $(DOCHDRS) \
									  CodeLoggingProcessor.cc \
									  PropertyProxy.cc

libclp_la_LIBADD					= 
AM_CPPFLAGS							= -DLOFARLOGGER_SUBPACKAGE=\"CLP\"

bin_PROGRAMS   						= CodeLoggingProcessor
  
CodeLoggingProcessor_SOURCES		= CLPMain.cc
CodeLoggingProcessor_LDADD		    = libclp.la $(LOFAR_DEPEND) \
									  $(top_builddir)/KVLogSys/src/libgcfkvlc.la
CodeLoggingProcessor_DEPENDENCIES	= libclp.la $(LOFAR_DEPEND) \
									  $(top_builddir)/KVLogSys/src/libgcfkvlc.la

BUILT_SOURCES 						=

EXTRA_DIST 							= $(sysconf_DATA)

sysconf_DATA						= CodeLoggingProcessor.log_prop
	
%.conf: %.conf.in
	cp $< $@

%.log_prop: %.log_prop.in
	cp $< $@

include $(top_srcdir)/Makefile.common

