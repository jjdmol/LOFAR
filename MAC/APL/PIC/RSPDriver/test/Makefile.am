
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src

# TESTS = \
# 	RSPTest \
# 	EPAStub

bin_PROGRAMS = \
	EPAStub

check_PROGRAMS = \
	RSPTest

RSPTest_SOURCES = \
	RSPTestSuite.h \
	RSPTest.h \
	RSPTest.cc

RSPTest_LDADD = \
	$(top_builddir)/src/librsp.la \
	-lsuite

EPAStub_SOURCES = \
	RSPTestSuite.h \
	EPAStub.h \
	EPAStub.cc

EPAStub_LDADD = \
	$(top_builddir)/src/libepa.la \
	-lsuite

BUILT_SOURCES = \
	RSPTest.conf \
	EPAStub.conf

sysconf_DATA = \
	EPAStub.conf

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
