bin_PROGRAMS 				= DigitalBoardControl

DigitalBoardControl_CPPFLAGS		= -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once

DigitalBoardControl_SOURCES		= ../Package__Version.cc \
								  DigitalBoardControl.cc \
								  DigitalBoardControlMain.cc 

DigitalBoardControl_LDADD		= $(LOFAR_DEPEND)
DigitalBoardControl_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= DigitalBoardControl.h \
							  PVSSDatapointDefs.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= 
sysconf_DATA		= DigitalBoardControl.conf

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
