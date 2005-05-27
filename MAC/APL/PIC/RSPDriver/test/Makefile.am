
if HAVE_SYS_CAPABILITY_H
CAPLIBS=-lnetraw -lcap
else
CAPLIBS=
endif

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
	RSPTest.h \
	RSPTest.cc

RSPTest_LDADD = \
	$(top_builddir)/src/librsp.la \
	-lsuite

EPAStub_SOURCES = \
	EPAStub.h \
	EPAStub.cc

EPAStub_LDADD = \
	$(top_builddir)/src/libepa.la \
	$(CAPLIBS) \
	-lsuite

BUILT_SOURCES = \
	RSPTest.conf \
	EPAStub.conf

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA = \
	EPAStub.conf

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
