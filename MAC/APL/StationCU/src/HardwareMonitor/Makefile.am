
AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

#	cp $*.ph $(top_builddir)/include/APL/StationCU/
	
BUILT_SOURCES = \
	EC_Protocol.ph \
	EC_Protocol.cc 


bin_PROGRAMS 				= HardwareMonitor

HardwareMonitor_CPPFLAGS	= -Wno-deprecated \
							  -fmessage-length=0 \
							  -fdiagnostics-show-location=once

HardwareMonitor_SOURCES		= ../Package__Version.cc \
							  HardwareMonitorMain.cc \
							  RSPMonitor.cc  \
							  TBBMonitor.cc \
							  ECMonitor.cc \
							  EC_Protocol.cc 

HardwareMonitor_LDADD		= $(LOFAR_DEPEND)
HardwareMonitor_DEPENDENCIES= $(LOFAR_DEPEND)

NOINSTHDRS 					= RSPMonitor.h \
							  TBBMonitor.h \
							  RCUConstants.h \
							  ECMonitor.h \
							  PVSSDatapointDefs.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

sysconf_DATA				= HardwareMonitor.conf

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
