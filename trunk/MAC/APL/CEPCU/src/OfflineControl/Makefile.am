bin_PROGRAMS 				= OfflineControl

OfflineControl_CPPFLAGS		= -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once

OfflineControl_SOURCES		= CEPApplicationManager.cc \
								OfflineControl.cc \
								OfflineControlMain.cc 

OfflineControl_LDADD		= $(LOFAR_DEPEND)
OfflineControl_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= CEPApplicationManager.h \
								OfflineControl.h \
							  	OfflineControlDefines.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= 
sysconf_DATA		= 

include $(top_srcdir)/Makefile.common
