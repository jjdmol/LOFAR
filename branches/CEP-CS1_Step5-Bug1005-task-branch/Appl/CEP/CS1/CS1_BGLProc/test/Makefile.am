
check_PROGRAMS = tBGL_Processing


AM_CPPFLAGS = $(EXTRA_CPPFLAGS)

AM_CXXFLAGS = \
	-I../../../src

tBGL_Processing_SOURCES =			\
tBGL_Processing.cc				\
../src/Allocator.cc				\
../src/BGL_Processing.cc			\
../src/BeamFormer.cc				\
../src/Correlator.cc				\
../src/CorrelatorAsm.S				\
../src/FFT_Asm.S				\
../src/FIR.cc					\
../src/FIR_Asm.S				\
../src/PPF.cc					\
../src/Transpose.cc

CCAS			= $(CC)
CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) $(EXTRA_CPPFLAGS)


TESTS   = 					\
tBGL_Processing.sh


include $(top_srcdir)/Makefile.common
