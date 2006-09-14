..common_top_builddir=@common_top_builddir@

check_PROGRAMS = tWH_BGL_Processing #tFFT


AM_CPPFLAGS = $(EXTRA_CPPFLAGS)

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

CCAS			= /bgl/BlueLight/ppcfloor/blrts-gnu/powerpc-bgl-blrts-gnu/bin/gcc
CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) $(EXTRA_CPPFLAGS)


TESTS   = 					\
tWH_BGL_Processing.sh


include $(top_srcdir)/Makefile.common
