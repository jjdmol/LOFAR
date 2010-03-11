bin_PROGRAMS 				= PythonControl

PythonControl_CPPFLAGS		= -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once

PythonControl_SOURCES		= PythonControl.cc \
							  PythonControlMain.cc 

PythonControl_LDADD		= $(LOFAR_DEPEND)
PythonControl_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= PythonControl.h \
							  PythonControlDefines.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= 
sysconf_DATA		= 

include $(top_srcdir)/Makefile.common
