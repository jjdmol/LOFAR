
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/test

#TESTS = \
#	CalTest

bin_PROGRAMS = \
	CalTest

check_PROGRAMS = \
	CalTest

CalTest_SOURCES = \
	CalTest.cc

CalTest_LDADD = \
	$(top_builddir)/src/libcal.la \
	-lsuite

BUILT_SOURCES = \
	CalTest.conf

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA = \
	CalTest.conf

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
