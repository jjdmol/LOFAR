bin_PROGRAMS 				= ObservationControl

ObservationControl_CPPFLAGS		= -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once

ObservationControl_SOURCES		= ObservationControl.cc \
								  ObservationControlMain.cc 

ObservationControl_LDADD		= $(LOFAR_DEPEND)
ObservationControl_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= ObservationControl.h \
							  ObservationControlDefines.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= 
#customPrepPVSSDB.ctl

sysconf_DATA		= ObservationControl.log_prop

%.log_prop: %.log_prop.in
	cp $< $@

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
