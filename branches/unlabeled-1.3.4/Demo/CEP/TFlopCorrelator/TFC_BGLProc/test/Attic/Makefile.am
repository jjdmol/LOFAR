..common_top_builddir=@common_top_builddir@

check_PROGRAMS = tWH_PPF tWH_Correlator


AM_CXXFLAGS = \
	-I../../../src

tWH_Correlator_SOURCES = 			\
tWH_Correlator.cc				\
../src/WH_Correlator.cc				\
../src/Correlator.S

tWH_PPF_SOURCES =	 			\
tWH_PPF.cc					\
../src/WH_PPF.cc				\
../src/FFT.S					\
../src/FIR.S

CCAS = gcc
CCASFLAGS = $(patsubst -q%,,$(CPPFLAGS)) -B/bgl/BlueLight/ppcfloor/blrts-gnu/powerpc-bgl-blrts-gnu/bin/

TESTS   = 					\
tWH_PPF.sh					\
tWH_Correlator.sh



TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(lofar_sharedir)/Makefile.common
