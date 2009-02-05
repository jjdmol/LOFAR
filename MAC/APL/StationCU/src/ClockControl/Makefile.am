bin_PROGRAMS 				= ClockControl

ClockControl_CPPFLAGS		= -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once

ClockControl_SOURCES		= ../Package__Version.cc \
								  ClockControl.cc \
								  ClockControlMain.cc 

ClockControl_LDADD		= $(LOFAR_DEPEND)
ClockControl_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= ClockControl.h \
							  PVSSDatapointDefs.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= 
sysconf_DATA		= 

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
