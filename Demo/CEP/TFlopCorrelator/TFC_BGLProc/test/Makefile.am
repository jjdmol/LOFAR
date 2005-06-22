..common_top_builddir=@common_top_builddir@

check_PROGRAMS = tWH_FFT TestFilter 


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

tWH_FFT_SOURCES =				\
tWH_FFT.cc				        \
../src/WH_FFT.cc

TESTS   = 					\
TestFilter.sh					\
tWH_FFT.sh					

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(lofar_sharedir)/Makefile.common
