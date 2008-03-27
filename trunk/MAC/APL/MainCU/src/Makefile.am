lib_LTLIBRARIES = libmaincu.la
libmaincu_la_SOURCES = Package__Version.cc

pkginclude_HEADERS = Package__Version.h

SUBDIRS = MACScheduler ObservationControl

bin_PROGRAMS = versionmaincu

versionmaincu_SOURCES      = versionmaincu.cc
versionmaincu_LDADD        = libmaincu.la
versionmaincu_DEPENDENCIES = libmaincu.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

