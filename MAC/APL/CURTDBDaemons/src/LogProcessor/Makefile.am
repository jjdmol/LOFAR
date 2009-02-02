# Executables
bin_PROGRAMS 				= LogProcessor

#LogProcessor_CPPFLAGS		= -DBOOST_DISABLE_THREADS \
#							  -Wno-deprecated \
#							  -fmessage-length=0 \
#							  -fdiagnostics-show-location=once

LogProcessor_SOURCES	= ../Package__Version.cc \
							  LogProcessor.cc \
							  LogProcessorMain.cc
LogProcessor_LDADD		= $(LOFAR_DEPEND) $(libdir)/liblog_protocol.a
LogProcessor_DEPENDENCIES=$(LOFAR_DEPEND)

bin_SCRIPTS					= 

NOINSTHDRS 					= LogProcessor.h

INSTHDRS 					= 

BUILT_SOURCES				= $(ph_HEADERS)

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 

sysconf_DATA				= LogProcessor.conf \
							  LogProcessor.log_prop 

include $(top_srcdir)/Makefile.common
