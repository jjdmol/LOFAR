common_top_builddir=@common_top_builddir@

check_PROGRAMS = tWH_Transpose TestFilter 

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

tWH_Transpose_SOURCES =				\
tWH_Transpose.cc				\
../src/WH_Transpose.cc				\
../src/DH_RSP.cc				\
../src/DH_SubBand.cc

TESTS   = 					\
TestFilter.sh					\
tWH_Transpose.sh

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(lofar_sharedir)/Makefile.common
