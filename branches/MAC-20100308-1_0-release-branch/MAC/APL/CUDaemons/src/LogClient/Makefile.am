# Executables
bin_PROGRAMS 				= LogClient

LogClient_SOURCES		= ../Package__Version.cc \
							  LogClient.cc \
							  LogClientMain.cc

bin_SCRIPTS					= 

INSTHDRS 					= LogClient.h

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

sysconf_DATA				= LogClient.conf

include $(top_srcdir)/Makefile.common
