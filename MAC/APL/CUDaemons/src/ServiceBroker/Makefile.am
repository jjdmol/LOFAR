# Executables
bin_PROGRAMS 				= ServiceBroker

ServiceBroker_CPPFLAGS		= -DBOOST_DISABLE_THREADS \
							  -Wno-deprecated \
							  -fmessage-length=0 \
							  -fdiagnostics-show-location=once

ServiceBroker_SOURCES		= ../Package__Version.cc \
							  ServiceBroker.cc \
							  ServiceBrokerMain.cc
ServiceBroker_LDADD			= $(LOFAR_DEPEND)
ServiceBroker_DEPENDENCIES	= $(LOFAR_DEPEND)

bin_SCRIPTS					= 

NOINSTHDRS 					= ServiceBroker.h

INSTHDRS 					= 

BUILT_SOURCES				= $(ph_HEADERS)

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= 

sysconf_DATA		= ServiceBroker.conf 

include $(top_srcdir)/Makefile.common
