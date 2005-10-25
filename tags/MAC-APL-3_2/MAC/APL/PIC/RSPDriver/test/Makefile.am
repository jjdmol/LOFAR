
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src

# TESTS = \
# 	RSPTest \
# 	EPAStub

bin_PROGRAMS = \
	EPAStub \
	RSPTest

#check_PROGRAMS = \
#	RSPTest

RSPTest_SOURCES = \
	RSPTest.h \
	RSPTest.cc

EPAStub_SOURCES = \
	EPAStub.h \
	EPAStub.cc

EPAStub_LDADD = \
	$(top_builddir)/src/libepa.la

BUILT_SOURCES = \
	RSPTest.conf \
	EPAStub.conf

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA = \
	EPAStub.conf \
	RSPTest.conf

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
