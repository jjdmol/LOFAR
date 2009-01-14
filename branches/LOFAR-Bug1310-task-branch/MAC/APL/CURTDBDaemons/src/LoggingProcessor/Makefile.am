# Executables
bin_PROGRAMS 				= LoggingProcessor

#LoggingProcessor_CPPFLAGS		= -DBOOST_DISABLE_THREADS \
#							  -Wno-deprecated \
#							  -fmessage-length=0 \
#							  -fdiagnostics-show-location=once

LoggingProcessor_SOURCES	= ../Package__Version.cc \
							  LoggingProcessor.cc \
							  LoggingProcessorMain.cc
LoggingProcessor_LDADD		= $(LOFAR_DEPEND) $(libdir)/liblog_protocol.a
LoggingProcessor_DEPENDENCIES=$(LOFAR_DEPEND)

bin_SCRIPTS					= 

NOINSTHDRS 					= LoggingProcessor.h

INSTHDRS 					= 

BUILT_SOURCES				= $(ph_HEADERS)

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 

sysconf_DATA				= LoggingProcessor.conf \
							  LoggingProcessor.log_prop 

include $(top_srcdir)/Makefile.common
