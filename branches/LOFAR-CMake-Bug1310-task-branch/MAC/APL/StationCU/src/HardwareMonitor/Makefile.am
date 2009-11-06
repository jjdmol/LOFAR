bin_PROGRAMS 				= HardwareMonitor

HardwareMonitor_CPPFLAGS	= -Wno-deprecated \
							  -fmessage-length=0 \
							  -fdiagnostics-show-location=once

HardwareMonitor_SOURCES		= ../Package__Version.cc \
							  HardwareMonitorMain.cc \
							  RSPMonitor.cc  \
							  TBBMonitor.cc 

HardwareMonitor_LDADD		= $(LOFAR_DEPEND)
HardwareMonitor_DEPENDENCIES= $(LOFAR_DEPEND)

NOINSTHDRS 					= RSPMonitor.h \
							  TBBMonitor.h \
							  RCUConstants.h \
							  PVSSDatapointDefs.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

sysconf_DATA				= 

include $(top_srcdir)/Makefile.common
