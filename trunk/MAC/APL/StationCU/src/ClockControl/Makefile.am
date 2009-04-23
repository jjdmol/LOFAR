bin_PROGRAMS 				= ClockControl clkctl

ClockControl_CPPFLAGS		= -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once \
								  -I$(top_builddir)/include
ClockControl_SOURCES		= ../Package__Version.cc \
								  ClockControl.cc \
								  ClockControlMain.cc 
ClockControl_LDADD			= $(top_builddir)/src/ClockProtocol/libclock_protocol.la $(LOFAR_DEPEND)
ClockControl_DEPENDENCIES	= $(LOFAR_DEPEND)


clkctl_SOURCES				= clkctl.cc
clkctl_LDADD				= $(top_builddir)/src/ClockProtocol/libclock_protocol.la $(LOFAR_DEPEND)
clkctl_DEPENDENCIES			= $(LOFAR_DEPEND)

NOINSTHDRS 					= ClockControl.h \
							  PVSSDatapointDefs.h \
							  clkctl.h

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
