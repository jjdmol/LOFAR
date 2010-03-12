AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/autogen $<
	cp $*.ph $(top_builddir)/include/MACIO/

%.cc: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/autogen $<

clean-local:
	rm -f *.ph

bin_PROGRAMS 				= versionmacio 
bin_SCRIPTS					= 
INSTHDRS 					= LOG_Protocol.ph KVT_Protocol.ph SB_Protocol.ph
NOINSTHDRS 					= 
BUILT_SOURCES				= LOG_Protocol.cc \
							  LOG_Protocol.ph \
							  KVT_Protocol.cc \
							  KVT_Protocol.ph \
							  SB_Protocol.cc \
							  SB_Protocol.ph

pkgincludedir				= $(includedir)/MACIO
pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)
DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)
EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

# Libraries
lib_LTLIBRARIES         	= liblog_protocol.la \
							  libkvt_protocol.la \
							  libmacio.la
liblog_protocol_la_SOURCES 	= LOG_Protocol.cc
libkvt_protocol_la_SOURCES 	= KVT_Protocol.cc
libmacio_la_SOURCES 		= Package__Version.cc GCF_Event.cc EventPort.cc KVTLogger.cc ProtocolDefs.cc SB_Protocol.cc

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 
sysconf_DATA				= 

versionmacio_SOURCES      = versionmacio.cc
versionmacio_LDADD        = libmacio.la
versionmacio_DEPENDENCIES = libmacio.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
