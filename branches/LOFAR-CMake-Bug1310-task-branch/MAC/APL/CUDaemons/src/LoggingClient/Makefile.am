# Executables
bin_PROGRAMS 				= LoggingClient

LoggingClient_SOURCES		= ../Package__Version.cc \
							  LoggingClient.cc \
							  LoggingClientMain.cc

bin_SCRIPTS					= 

INSTHDRS 					= LoggingClient.h

NOINSTHDRS 					= 

BUILT_SOURCES				= 

pkgincludedir				= $(includedir)/APL/CUDaemons

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

# Libraries
lib_LTLIBRARIES         	= 

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 

sysconf_DATA				= LoggingClient.conf \
							  LoggingClient.log_prop 

include $(top_srcdir)/Makefile.common
