bin_PROGRAMS 				= OnlineControl

OnlineControl_CPPFLAGS		= -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once

OnlineControl_SOURCES		= CEPApplMgr.cc \
								OnlineControl.cc \
								OnlineControlMain.cc 

OnlineControl_LDADD		= $(LOFAR_DEPEND)
OnlineControl_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= CEPApplMgr.h \
								OnlineControl.h \
							  	OnlineControlDefines.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= 
sysconf_DATA		= 

include $(top_srcdir)/Makefile.common
