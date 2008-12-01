pkginclude_HEADERS = Package__Version.h

AM_CPPFLAGS = -I$(top_builddir)/include

MATLAB_COMPILER = mcc
lib%.so : %.m
	$(MATLAB_COMPILER) -v -W cpplib:lib$* -T link:lib $<
	mkdir -p $(pkgincludedir)/
	cp lib$*.h  $(pkgincludedir)/$*.h
	cp lib$*.so $(libdir)/

#libMatlabCal.so: calibrate.m
#	$(MATLAB_COMPILER) -v -W cpplib:libMatlabCal -T link:lib $<
#	mkdir -p $(pkgincludedir)/
#	cp libMatlabCal.h  $(pkgincludedir)/
#	cp libMatlabCal.so $(libdir)/

EXTRA_DIST = 

BUILT_SOURCES	= liblba_calibration.so

#lib_LTLIBRARIES	= libcalibrate.la
#libcalibrate_la_SOURCES = calibrate.m 


pkgincludedir = $(includedir)/APL/LBA_Calibration
#pkginclude_HEADERS = $(BUILT_SOURCES)

bin_PROGRAMS = versionlba_calibration 

versionlba_calibration_SOURCES      = Package__Version.cc versionlba_calibration.cc
versionlba_calibration_LDADD        = 
versionlba_calibration_DEPENDENCIES = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
