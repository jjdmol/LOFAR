INSTALL=/usr/bin/install -C

pkgincludedir = $(includedir)/GCF/LogSys
pkginclude_HEADERS =
	
DOCHDRS	=	$(pkginclude_HEADERS) \
	CodeLoggingProcessor.h \
	PropertyProxy.h
   
noinst_LTLIBRARIES 	= libclp.la

libclp_la_SOURCES	= $(DOCHDRS) \
	CodeLoggingProcessor.cc \
	PropertyProxy.cc

libclp_la_LIBADD		= 
libclp_la_CPPFLAGS= -DLOFARLOGGER_SUBPACKAGE=\"CLP\"


bin_PROGRAMS   		= CodeLoggingProcessor
  
CodeLoggingProcessor_SOURCES		  = CLPMain.cc
CodeLoggingProcessor_LDADD		    = libclp.la $(LOFAR_DEPEND) \
	$(top_builddir)/KVLogSys/src/libgcfkvlc.la
CodeLoggingProcessor_DEPENDENCIES	= libclp.la $(LOFAR_DEPEND) \
	$(top_builddir)/KVLogSys/src/libgcfkvlc.la
CodeLoggingProcessor_CPPFLAGS			= -DLOFARLOGGER_SUBPACKAGE=\"CLP\" \
	-I$(top_srcdir)/include

configfilesdir=$(bindir)
configfiles_DATA= \
	CodeLoggingProcessor.conf \
	CodeLoggingProcessor.log_prop
	
%.conf: %.conf.in
	cp $< $@

%.log_prop: %.log_prop.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

