
#
# if USE_CAL_THREAD is defined in Calibration/src/Makefile.am it should be
# defined here as well and vice versa
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/test \
	-DUSE_CAL_THREAD

#	CheckWeightsTest 
#	CheckWeightsTest2 

TESTS = 
#	avtstub.sh 
#	BeamServerTest 

#bin_PROGRAMS = BeamServerTest
#	AVTStub 
#	EPATest 
#	SweepTest

bin_SCRIPTS = \
	beamctltest

#	BeamServerTest 
#	CheckWeightsTest2 

#check_PROGRAMS = BeamServerTest

#BeamServerTest_SOURCES	= BSTest.h  BeamServerTest.cc
#BeamServerTest_LDADD 	= $(top_builddir)/src/libabsdevel.la

#CheckWeightsTest_SOURCES = CheckWeightsTest.cc
#CheckWeightsTest_LDADD = $(top_builddir)/src/libabs.la $(top_builddir)/src/libabsdevel.la

#CheckWeightsTest2_SOURCES = CheckWeightsTest2.cc
#CheckWeightsTest2_LDADD = $(top_builddir)/src/libabs.la $(top_builddir)/src/libabsdevel.la

#AVTStub_SOURCES 	= AVTStub.h AVTStub.cc
#AVTStub_LDADD 		= $(top_builddir)/src/libabsdevel.la

#EPATest_SOURCES 	= EPATest.h EPATest.cc
#EPATest_LDADD 		= $(top_builddir)/src/libabsdevel.la

#SweepTest_SOURCES 	=  SweepTest.h  SweepTest.cc
#SweepTest_LDADD 	= $(top_builddir)/src/libabsdevel.la

BUILT_SOURCES = \
	AVTStub.conf \
	EPATest.conf \
	SweepTest.conf \
	BeamServerTest.conf

EXTRA_DIST = \
	$(BUILT_SOURCES) \
	avtstub.sh \
	weights.dat

sysconf_DATA = \
	AVTStub.conf \
	EPATest.conf \
	SweepTest.conf \
	BeamServerTest.conf

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common

