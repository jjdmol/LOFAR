..common_top_builddir=@common_top_builddir@

check_PROGRAMS = tWH_FFT tWH_FIR tWH_Correlator tPPFilter


AM_CXXFLAGS = \
	-I../../../src

tWH_FFT_SOURCES =				\
tWH_FFT.cc				        \
../src/WH_FFT.cc

tWH_FIR_SOURCES = 				\
tWH_FIR.cc					\
../src/WH_FIR.cc

tWH_Correlator_SOURCES = 			\
tWH_Correlator.cc				\
../src/WH_Correlator.cc

tPPFilter_SOURCES = 				\
tPPFilter.cc					\
../src/WH_FIR.cc				\
../src/WH_FFT.cc

TESTS   = 					\
tWH_FFT.sh					\
tWH_FIR.sh					\
tWH_Correlator.sh



TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(lofar_sharedir)/Makefile.common
