
AM_CXXFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/test \
	-I$(top_srcdir)/../../APLCommon/src

TESTS = \
	BeamServerTest avtstub.sh

noinst_LTLIBRARIES = libunittest.la

libunittest_la_SOURCES = \
	$(top_srcdir)/../../APLCommon/src/test.cpp \
	$(top_srcdir)/../../APLCommon/src/suite.cpp

check_PROGRAMS = \
	BeamServerTest \
	AVTStub \
	EPAStub

BeamServerTest_SOURCES = \
	BeamServerTest.cc

BeamServerTest_LDADD = \
	$(top_builddir)/src/libabs.la \
	$(top_builddir)/test/libunittest.la

AVTStub_SOURCES = \
	ABSAVTStub.h \
	ABSAVTStub.cc

AVTStub_LDADD = \
	$(top_builddir)/src/libabs.la \
	$(top_builddir)/test/libunittest.la \
	 -ltm -lgcfcommon

EPAStub_SOURCES = \
	ABSEPAStub.h \
	ABSEPAStub.cc

EPAStub_LDADD = \
	$(top_builddir)/src/libabs.la \
	$(top_builddir)/test/libunittest.la \
	-ltm -lgcfcommon

BUILT_SOURCES = \
	log4cplus.properties \
	abstest.ns \
	abstest.top

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common