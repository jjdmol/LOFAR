DOCHDRS		 	= 			\
ArrayConfig.h					\
DH_SampleC.h					\
DH_SampleR.h					\
DH_SubBandSel.h					\
DH_Weight.h					\
DataGenConfig.h					\
FFTW.h						\
FilterBank.h					\
Modulate.h					\
PhaseShift.h					\
Signal.h					\
Source.h					\
StationSim.h					\
Trajectory.h					\
WH_AWE.h					\
WH_AddSignals.h					\
WH_BandSep.h					\
WH_BeamFormer.h					\
WH_Calibration.h				\
WH_Cancel.h					\
WH_CreateSource.h				\
WH_Detection.h					\
WH_Modulate.h					\
WH_PhaseShift.h					\
WH_ReadSignal.h					\
WH_Selector.h					\
WH_SubBandSel.h


lib_LTLIBRARIES 	= libstationsim.la

libstationsim_la_SOURCES 	= $(DOCHDRS)	\
ArrayConfig.cc					\
DH_SampleC.cc					\
DH_SampleR.cc					\
DH_SubBandSel.cc				\
DH_Weight.cc					\
DataGenConfig.cc				\
FFTW.cc						\
FilterBank.cc					\
Modulate.cc					\
PhaseShift.cc					\
Signal.cc					\
Source.cc					\
StationSim.cc					\
Trajectory.cc					\
WH_AWE.cc					\
WH_AddSignals.cc				\
WH_BandSep.cc					\
WH_BeamFormer.cc				\
WH_Calibration.cc				\
WH_Cancel.cc					\
WH_CreateSource.cc				\
WH_Detection.cc					\
WH_Modulate.cc					\
WH_PhaseShift.cc				\
WH_ReadSignal.cc				\
WH_Selector.cc					\
WH_SubBandSel.cc

bin_PROGRAMS		= Simulate
Simulate_SOURCES 	= Simulate.cc Simulate.in

Simulate_LDADD		= libstationsim.la
Simulate_DEPENDENCIES	= libstationsim.la $(LOFAR_DEPEND)

#EXTRA_DIST		=			\
#Beam.m						\
#spectralfft.m					\
#Simulate.out

include $(lofar_sharedir)/Makefile.common
