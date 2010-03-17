# Executables
bin_PROGRAMS 				= CTStartDaemon

CTStartDaemon_CPPFLAGS		= -DBOOST_DISABLE_THREADS \
							  -Wno-deprecated \
							  -fmessage-length=0 \
							  -fdiagnostics-show-location=once

CTStartDaemon_SOURCES		= ../Package__Version.cc \
							  CTStartDaemon.cc \
							  CTStartDaemonMain.cc
CTStartDaemon_LDADD			= $(LOFAR_DEPEND) 
CTStartDaemon_DEPENDENCIES	= $(LOFAR_DEPEND)

bin_SCRIPTS					= startController.sh

NOINSTHDRS 					= CTStartDaemon.h

INSTHDRS 					= 

BUILT_SOURCES				= $(ph_HEADERS)

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= 

sysconf_DATA		= CTStartDaemon.conf

include $(top_srcdir)/Makefile.common
