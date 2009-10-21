
check_PROGRAMS = tCN_Processing tBeamForming tPencilBeamFormer


AM_CPPFLAGS = $(EXTRA_CPPFLAGS)

AM_CXXFLAGS = \
	-I../../../src

tCN_Processing_SOURCES =			\
tCN_Processing.cc				\
../src/BandPass.cc				\
../src/BeamFormerAsm.S				\
../src/CN_Processing.cc		        	\
../src/Correlator.cc				\
../src/CorrelatorAsm.S				\
../src/FFT_Asm.S				\
../src/FIR.cc					\
../src/FIR_Asm.S				\
../src/LocationInfo.cc				\
../src/PPF.cc					\
../src/PencilBeams.cc				\
../src/Stokes.cc				\
../src/AsyncCommunication.cc			\
../src/AsyncTranspose.cc

tBeamForming_SOURCES =				\
tBeamForming.cc					\
../src/BeamFormerAsm.S

tPencilBeamFormer_SOURCES =			\
tPencilBeamFormer.cc				\
../src/PencilBeams.cc                           \
../src/BeamFormerAsm.S

CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) $(EXTRA_CPPFLAGS)


TESTS   = 					\
tCN_Processing.sh                               \
tPencilBeamFormer.sh


include $(top_srcdir)/Makefile.common
