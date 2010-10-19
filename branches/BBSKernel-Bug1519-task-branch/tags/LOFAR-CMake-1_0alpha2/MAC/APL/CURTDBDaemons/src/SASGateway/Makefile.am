# Executables
bin_PROGRAMS 				= SASGateway

#SASGateway_CPPFLAGS		= -DBOOST_DISABLE_THREADS \
#							  -Wno-deprecated \
#							  -fmessage-length=0 \
#							  -fdiagnostics-show-location=once

SASGateway_SOURCES		= ../Package__Version.cc \
							  SASGateway.cc \
							  SASGatewayMain.cc
SASGateway_LDADD		= $(LOFAR_DEPEND)
SASGateway_DEPENDENCIES = $(LOFAR_DEPEND)

bin_SCRIPTS					= 

NOINSTHDRS 					= SASGateway.h

INSTHDRS 					= 

BUILT_SOURCES				= $(ph_HEADERS)

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 

sysconf_DATA				= SASGateway.conf

include $(top_srcdir)/Makefile.common
