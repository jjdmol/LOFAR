
AM_CXXFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src

# TESTS = \
# 	RSPTest \
# 	EPAStub

check_PROGRAMS = \
	RSPTest \
	EPAStub

RSPTest_SOURCES = \
	RSPTestSuite.h \
	RSPTest.h \
	RSPTest.cc

RSPTest_LDADD = \
	$(top_builddir)/src/librsp.la \
	-lsuite -lgcf

EPAStub_SOURCES = \
	RSPTestSuite.h \
	EPAStub.h \
	EPAStub.cc

EPAStub_LDADD = \
	$(top_builddir)/src/libepa.la \
	-lsuite -lgcf

BUILT_SOURCES = \
	log4cplus.properties \
	test.ns \
	test.top

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
