AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/APL/CUDaemons/

%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

clean-local:
	rm -f *.ph

# Executables
bin_PROGRAMS 				= LoggingClient

LoggingClient_SOURCES		= LoggingClient.cc \
							  LoggingClientMain.cc
LoggingClient_LDADD			= $(LOFAR_DEPEND) liblog_protocol.la
LoggingClient_DEPENDENCIES	= $(LOFAR_DEPEND) liblog_protocol.la

bin_SCRIPTS					= 

INSTHDRS 					= LoggingClient.h Log_Protocol.ph

NOINSTHDRS 					= 

BUILT_SOURCES				= Log_Protocol.cc \
							  Log_Protocol.ph

pkgincludedir				= $(includedir)/APL/CUDaemons

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

# Libraries
lib_LTLIBRARIES         	= liblog_protocol.la

liblog_protocol_la_SOURCES 	= Log_Protocol.cc

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 

sysconf_DATA				= LoggingClient.conf \
							  LoggingClient.log_prop 

include $(top_srcdir)/Makefile.common
