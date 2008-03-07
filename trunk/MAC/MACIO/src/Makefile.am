AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/APLProtocol/

%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

clean-local:
	rm -f *.ph

bin_PROGRAMS 				= 
bin_SCRIPTS					= 
INSTHDRS 					= LOG_Protocol.ph KVT_Protocol.ph 
NOINSTHDRS 					= 
BUILT_SOURCES				= LOG_Protocol.cc \
							  LOG_Protocol.ph \
							  KVT_Protocol.cc \
							  KVT_Protocol.ph

pkgincludedir				= $(includedir)/APLProtocol
pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)
DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)
EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

# Libraries
lib_LTLIBRARIES         	= liblog_protocol.la libkvt_protocol.la
liblog_protocol_la_SOURCES 	= LOG_Protocol.cc
libkvt_protocol_la_SOURCES 	= KVT_Protocol.cc

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 
sysconf_DATA				= 

include $(top_srcdir)/Makefile.common
