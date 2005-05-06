common_top_builddir=@common_top_builddir@

check_PROGRAMS = TestFilter

AM_CXXFLAGS = \
	-I../../../src

TestFilter_SOURCES = 				\
WH_FilterInput.cc				\
WH_FilterOutput.cc				\
AH_TestFilter.cc				\
TestFilter.cc					\
../src/WH_SubBand.cc				\
../src/DH_SubBand.cc				\
../src/DH_CorrCube.cc

TESTS   = 					\
TestFilter.sh

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(lofar_sharedir)/Makefile.common
