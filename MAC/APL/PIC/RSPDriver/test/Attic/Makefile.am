
AM_CXXFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/test \
	-I$(top_srcdir)/../../APLCommon/src

TESTS = 

# 	BeamServerTest \
# 	CheckWeightsTest \
# 	CheckWeightsTest2 \
# 	avtstub.sh


# noinst_LTLIBRARIES = libunittest.la

# libunittest_la_SOURCES = \
# 	$(top_srcdir)/../../APLCommon/src/test.cpp \
# 	$(top_srcdir)/../../APLCommon/src/suite.cpp

check_PROGRAMS =

# 	BeamServerTest \
# 	CheckWeightsTest \
# 	CheckWeightsTest2 \
# 	AVTStub \
# 	EPAStub \
# 	EPATest \
# 	SweepTest \
# 	SetFreq

BeamServerTest_SOURCES = \
	BeamServerTest.cc

BeamServerTest_LDADD = \
	$(top_builddir)/src/libabs.la \
	$(top_builddir)/test/libunittest.la

CheckWeightsTest_SOURCES = \
	CheckWeightsTest.cc

CheckWeightsTest_LDADD = \
	$(top_builddir)/src/libabs.la \
	$(top_builddir)/test/libunittest.la

CheckWeightsTest2_SOURCES = \
	CheckWeightsTest2.cc

CheckWeightsTest2_LDADD = \
	$(top_builddir)/src/libabs.la \
	$(top_builddir)/test/libunittest.la

AVTStub_SOURCES = \
	ABSAVTStub.h \
	ABSAVTStub.cc

AVTStub_LDADD = \
	$(top_builddir)/src/libabs.la \
	$(top_builddir)/test/libunittest.la \
	 -ltm -lgcfcommon

EPATest_SOURCES = \
	ABSEPATest.h \
	ABSEPATest.cc

EPATest_LDADD = \
	$(top_builddir)/src/libabs.la \
	$(top_builddir)/test/libunittest.la \
	 -ltm -lgcfcommon

SweepTest_SOURCES = \
	ABSSweepTest.h \
	ABSSweepTest.cc

SweepTest_LDADD = \
	$(top_builddir)/src/libabs.la \
	$(top_builddir)/test/libunittest.la \
	 -ltm -lgcfcommon

SetFreq_SOURCES = \
	ABSSetFreq.h \
	ABSSetFreq.cc

SetFreq_LDADD = \
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

# BUILT_SOURCES = \
# 	log4cplus.properties \
# 	abstest.ns \
# 	abstest.top

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
