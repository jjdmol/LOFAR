bin_PROGRAMS 				= TBBControl

TBBControl_CPPFLAGS			= -Wno-deprecated \
							  -fmessage-length=0 \
							  -fdiagnostics-show-location=once

TBBControl_SOURCES			= ../Package__Version.cc \
							  TBBControl.cc \
							  TBBObservation.cc \
							  TBBControlMain.cc 
TBBControl_LDADD			= $(LOFAR_DEPEND)
TBBControl_DEPENDENCIES		= $(LOFAR_DEPEND)

NOINSTHDRS 					= TBBControl.h \
							  TBBObservation.h  \
							  PVSSDatapointDefs.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 
sysconf_DATA				= TBBControl.conf

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
