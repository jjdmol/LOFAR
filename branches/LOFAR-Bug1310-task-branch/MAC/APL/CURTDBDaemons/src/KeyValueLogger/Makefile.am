# Executables
bin_PROGRAMS 				= KeyValueLogger

#KeyValueLogger_CPPFLAGS		= -DBOOST_DISABLE_THREADS \
#							  -Wno-deprecated \
#							  -fmessage-length=0 \
#							  -fdiagnostics-show-location=once

KeyValueLogger_SOURCES		= ../Package__Version.cc \
							  KeyValueLogger.cc \
							  KeyValueLoggerMain.cc
KeyValueLogger_LDADD		= $(LOFAR_DEPEND) $(libdir)/libkvt_protocol.a
KeyValueLogger_DEPENDENCIES = $(LOFAR_DEPEND)

bin_SCRIPTS					= 

NOINSTHDRS 					= KeyValueLogger.h

INSTHDRS 					= 

BUILT_SOURCES				= $(ph_HEADERS)

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 

sysconf_DATA				= KeyValueLogger.conf

include $(top_srcdir)/Makefile.common
