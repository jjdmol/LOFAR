
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/test

#TESTS = \
#	CalTest \
#	ITSCalTest

bin_PROGRAMS = \
	CalTest \
	ITSCalTest

check_PROGRAMS = \
	CalTest \
	ITSCalTest

CalTest_SOURCES = \
	CalTest.cc

CalTest_LDADD = \
	$(top_builddir)/src/libcal.la \
	-lsuite

ITSCalTest_SOURCES = \
	ITSCalTest.cc

ITSCalTest_LDADD = \
	$(top_builddir)/src/libcal.la \
	-lsuite

BUILT_SOURCES = \
	CalTest.conf \
	ITSCalTest.conf

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA = \
	CalTest.conf \
	ITSCalTest.conf

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
