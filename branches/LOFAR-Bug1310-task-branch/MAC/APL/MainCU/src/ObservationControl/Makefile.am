bin_PROGRAMS 				= ObservationControl

ObservationControl_CPPFLAGS		= -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once

ObservationControl_SOURCES		= ../Package__Version.cc \
							 	  ObservationControl.cc \
								  ObservationControlMain.cc 

ObservationControl_LDADD		= $(LOFAR_DEPEND)
ObservationControl_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= ObservationControl.h \
							  ObservationControlDefines.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

include $(top_srcdir)/Makefile.common
