
AM_CXXFLAGS = -I$(top_srcdir)/src

check_PROGRAMS = BeamServerTest

BeamServerTest_SOURCES = \
	BeamServerTest.cc

BeamServerTest_LDADD = \
	$(top_builddir)/src/libabs.la

TESTS = \
	BeamServerTest

include $(lofar_sharedir)/Makefile.common