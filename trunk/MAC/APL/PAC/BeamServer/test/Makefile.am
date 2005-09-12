
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/test

#	BeamServerTest 
#	CheckWeightsTest 
#	CheckWeightsTest2 

TESTS = \
	avtstub.sh \
	CheckWeightsTest

bin_PROGRAMS = \
	AVTStub \
	EPATest \
	SweepTest \
	CheckWeightsTest

#	BeamServerTest 
#	CheckWeightsTest2 

check_PROGRAMS = \
	CheckWeightsTest

BeamServerTest_SOURCES = \
# 	BSTest.h \
# 	BeamServerTest.cc

# BeamServerTest_LDADD = \
# 	$(top_builddir)/src/libabs.la \
# 	-lsuite

CheckWeightsTest_SOURCES = \
	CheckWeightsTest.cc

CheckWeightsTest_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite

# CheckWeightsTest2_SOURCES = \
# 	CheckWeightsTest2.cc

# CheckWeightsTest2_LDADD = \
# 	$(top_builddir)/src/libabs.la \
# 	-lsuite

AVTStub_SOURCES = \
	AVTStub.h \
	AVTStub.cc

AVTStub_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite

EPATest_SOURCES = \
	EPATest.h \
	EPATest.cc

EPATest_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite

SweepTest_SOURCES = \
	SweepTest.h \
	SweepTest.cc

SweepTest_LDADD = \
	$(top_builddir)/src/libabs.la \
	-lsuite

BUILT_SOURCES = \
	AVTStub.conf \
	EPATest.conf \
	SweepTest.conf

EXTRA_DIST = \
	$(BUILT_SOURCES) \
	avtstub.sh \
	weights.dat

sysconf_DATA = \
	AVTStub.conf \
	EPATest.conf \
	SweepTest.conf

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

#include $(lofar_sharedir)/Makefile.common
include ../Makefile.common
