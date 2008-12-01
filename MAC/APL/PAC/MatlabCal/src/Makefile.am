
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

BUILT_SOURCES	= libcalibrate.so

#lib_LTLIBRARIES	= libcalibrate.la
#libcalibrate_la_SOURCES = calibrate.m 


pkgincludedir = $(includedir)/APL/MatlabCal
#pkginclude_HEADERS = $(BUILT_SOURCES)

include $(top_srcdir)/Makefile.common
