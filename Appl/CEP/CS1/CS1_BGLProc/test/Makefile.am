
check_PROGRAMS = tWH_BGL_Processing transpose


AM_CPPFLAGS = $(EXTRA_CPPFLAGS)

AM_CXXFLAGS = \
	-I../../../src

tWH_BGL_Processing_SOURCES =			\
tWH_BGL_Processing.cc				\
../src/WH_BGL_Processing.cc			\
../src/Correlator.S				\
../src/FIR.S					\
../src/FFT.S

transpose_SOURCES =				\
transpose.cc					

CCAS			= $(CC)
CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) $(EXTRA_CPPFLAGS)


TESTS   = 					\
tWH_BGL_Processing.sh


include $(top_srcdir)/Makefile.common
