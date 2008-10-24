bin_PROGRAMS 				= StationControl

StationControl_CPPFLAGS		= -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once

StationControl_SOURCES		= ../Package__Version.cc \
							  StationControl.cc \
							  ActiveObs.cc \
							  StationControlMain.cc 

StationControl_LDADD		= $(LOFAR_DEPEND)
StationControl_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= StationControl.h \
							  ActiveObs.h \
							  PVSSDatapointDefs.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	=
sysconf_DATA		= StationControl.conf

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
