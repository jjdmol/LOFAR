bin_PROGRAMS 				= CalibrationControl

CalibrationControl_CPPFLAGS		= -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once

CalibrationControl_SOURCES		= ../Package__Version.cc \
							  	  CalibrationControl.cc \
								  CalibrationControlMain.cc 

CalibrationControl_LDADD		= $(LOFAR_DEPEND)
CalibrationControl_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= CalibrationControl.h \
							  PVSSDatapointDefs.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= 
sysconf_DATA		= CalibrationControl.conf 

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
