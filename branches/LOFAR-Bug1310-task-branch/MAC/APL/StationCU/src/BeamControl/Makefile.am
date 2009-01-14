bin_PROGRAMS 				= BeamControl

BeamControl_CPPFLAGS		= -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once

BeamControl_SOURCES			= ../Package__Version.cc \
							  BeamControl.cc \
							  BeamControlMain.cc 

BeamControl_LDADD			= $(LOFAR_DEPEND)
BeamControl_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= BeamControl.h \
							  PVSSDatapointDefs.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

include $(top_srcdir)/Makefile.common
