bin_PROGRAMS 				= SoftwareMonitor

SoftwareMonitor_CPPFLAGS	= -Wno-deprecated \
							  -fmessage-length=0 \
							  -fdiagnostics-show-location=once

SoftwareMonitor_SOURCES		= ../Package__Version.cc \
							  SoftwareMonitorMain.cc \
							  SoftwareMonitor.cc  

SoftwareMonitor_LDADD		= $(LOFAR_DEPEND)
SoftwareMonitor_DEPENDENCIES= $(LOFAR_DEPEND)

NOINSTHDRS 					= SoftwareMonitor.h \
							  PVSSDatapointDefs.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

sysconf_DATA				= SoftwareMonitor.conf

include $(top_srcdir)/Makefile.common
