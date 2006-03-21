..common_top_builddir=@common_top_builddir@

check_PROGRAMS = tWH_BGL_Processing #tFFT


AM_CXXFLAGS = \
	-I../../../src

tWH_BGL_Processing_SOURCES =			\
tWH_BGL_Processing.cc				\
../src/WH_BGL_Processing.cc			\
../src/Correlator.S				\
../src/FIR.S

#tFFT_SOURCES =					\
#tFFT.cc						\
#../src/FFT.S


CCAS = gcc
CCASFLAGS = $(patsubst -q%,,$(CPPFLAGS)) -B/bgl/BlueLight/ppcfloor/blrts-gnu/powerpc-bgl-blrts-gnu/bin/

TESTS   = 					\
tWH_BGL_Processing.sh

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(top_srcdir)/Makefile.common
