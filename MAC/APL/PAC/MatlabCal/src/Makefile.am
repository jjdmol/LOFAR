
AM_CPPFLAGS = -I$(top_builddir)/include

MATLAB_COMPILER = mcc
libMatlabCal.so: calibrate.m
	$(MATLAB_COMPILER) -v -W cpplib:libMatlabCal -T link:lib $<

EXTRA_DIST = 

BUILT_SOURCES = libMatlabCal.so

#pkgincludedir=$(includedir)/APL/CAL_Protocol
#pkginclude_HEADERS = \ $(BUILT_SOURCES)

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
