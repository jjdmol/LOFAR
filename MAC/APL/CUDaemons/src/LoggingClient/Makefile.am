AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

clean-local:
	rm -f *.ph

# Executables
bin_PROGRAMS 				= LoggingClient

LoggingClient_SOURCES		= LoggingClient.cc \
							  LoggingClientMain.cc \
							  Log_Protocol.cc
LoggingClient_LDADD			= $(LOFAR_DEPEND) 
LoggingClient_DEPENDENCIES	= $(LOFAR_DEPEND)

bin_SCRIPTS					= 

NOINSTHDRS 					= LoggingClient.h

INSTHDRS 					= 

BUILT_SOURCES				= Log_Protocol.ph

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 

sysconf_DATA				= LoggingClient.conf \
							  LoggingClient.log_prop 

include $(top_srcdir)/Makefile.common
