AM_CPPFLAGS = -DUSES_NEW_IOSTREAMS


lib_LTLIBRARIES 		= libstationcu.la
libstationcu_la_SOURCES = Package__Version.cc

#pkginclude_HEADERS = Package__Version.h

SUBDIRS = BeamControl CalibrationControl ClockProtocol ClockControl StationControl TBBControl HardwareMonitor

bin_PROGRAMS = versionstationcu

versionstationcu_SOURCES      = versionstationcu.cc
versionstationcu_LDADD        = libstationcu.la
versionstationcu_DEPENDENCIES = libstationcu.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

