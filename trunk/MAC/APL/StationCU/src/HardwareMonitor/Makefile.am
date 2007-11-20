bin_PROGRAMS 				= HardwareMonitor

HardwareMonitor_CPPFLAGS	= -Wno-deprecated \
							  -fmessage-length=0 \
							  -fdiagnostics-show-location=once

HardwareMonitor_SOURCES		= HardwareMonitorMain.cc \
							  RSPMonitor.cc  \
							  TBBMonitor.cc 

HardwareMonitor_LDADD		= $(LOFAR_DEPEND)
HardwareMonitor_DEPENDENCIES= $(LOFAR_DEPEND)

NOINSTHDRS 					= RSPMonitor.h \
							  TBBMonitor.h \
							  RCUConstants.h \
							  StationPermDatapointDefs.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

sysconf_DATA				= HardwareMonitor.log_prop

include $(top_srcdir)/Makefile.common
