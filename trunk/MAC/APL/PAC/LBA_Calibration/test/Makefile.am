
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/test

sbin_PROGRAMS = \
	tCalibrate

tCalibrate_SOURCES 	= $(BUILT_SOURCES) tCalibrate.cc
tCalibrate_LDADD	= -llba_calibration -L../src/ $(LOFAR_DEPEND)
tCalibrate_DEPENDENCIES	= $(LOFAR_DEPEND)

BUILT_SOURCES = 

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA = 

include $(top_srcdir)/Makefile.common
