bin_PROGRAMS = avitest

TESTS = #avitest

avitest_CPPFLAGS = \
	-I$(top_srcdir)/../GCF/GCFCommon/src \
	-I$(top_srcdir)/VIC/VirtualInstrument/src \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

avitest_SOURCES	= \
	AVITestAnswer.cc \
	AVITestTask.cc \
	AVITestDriverTask.cc \
	AVITestMain.cc

avitest_LDADD = -lsuite

NOINSTHDRS =

INSTHDRS = \
	AVITestAnswer.h \
	AVITestTask.h \
	AVITestDriverTask.h

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS =       $(pkginclude_HEADERS)

include_HEADERS =

BUILT_SOURCES =         \
	avitest.conf \
	avitest.log_prop

EXTRA_DIST = $(BUILT_SOURCES) $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA =

sysconf_DATA= $(BUILT_SOURCES)

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
