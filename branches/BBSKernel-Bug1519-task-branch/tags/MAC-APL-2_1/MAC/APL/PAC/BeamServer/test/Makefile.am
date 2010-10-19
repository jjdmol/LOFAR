
AM_CXXFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/test

TESTS = \
	BeamServerTest \
	CheckWeightsTest \
	CheckWeightsTest2 \
	avtstub.sh

check_PROGRAMS = \
	BeamServerTest \
	CheckWeightsTest \
	CheckWeightsTest2 \
	AVTStub \
	EPAStub \
	EPATest \
	SweepTest \
	SetFreq

BeamServerTest_SOURCES = \
	BeamServerTest.cc

BeamServerTest_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite

CheckWeightsTest_SOURCES = \
	CheckWeightsTest.cc

CheckWeightsTest_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite

CheckWeightsTest2_SOURCES = \
	CheckWeightsTest2.cc

CheckWeightsTest2_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite

AVTStub_SOURCES = \
	ABSAVTStub.h \
	ABSAVTStub.cc

AVTStub_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite

EPATest_SOURCES = \
	ABSEPATest.h \
	ABSEPATest.cc

EPATest_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite

SweepTest_SOURCES = \
	ABSSweepTest.h \
	ABSSweepTest.cc

SweepTest_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite -lgcflite

SetFreq_SOURCES = \
	ABSSetFreq.h \
	ABSSetFreq.cc

SetFreq_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite -lgcflite

EPAStub_SOURCES = \
	ABSEPAStub.h \
	ABSEPAStub.cc

EPAStub_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite	-lgcflite

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
