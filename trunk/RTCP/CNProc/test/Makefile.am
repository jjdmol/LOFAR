
check_PROGRAMS = tCN_Processing


AM_CPPFLAGS = $(EXTRA_CPPFLAGS)

AM_CXXFLAGS = \
	-I../../../src

tCN_Processing_SOURCES =			\
tCN_Processing.cc				\
../src/BandPass.cc				\
../src/CN_Processing.cc		        	\
../src/Correlator.cc				\
../src/CorrelatorAsm.S				\
../src/FFT_Asm.S				\
../src/FIR.cc					\
../src/FIR_Asm.S				\
../src/LocationInfo.cc				\
../src/PPF.cc					\
../src/Transpose.cc

CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) $(EXTRA_CPPFLAGS)


TESTS   = 					\
tCN_Processing.sh


include $(top_srcdir)/Makefile.common
