DOCHDRS		 	= 			\
ArrayConfig.h					\
DH_SampleC.h					\
DH_SampleR.h					\
DH_SubBandSel.h					\
DataGenConfig.h					\
FFTW.h						\
FilterBank.h					\
Modulate.h					\
PhaseShift.h					\
Signal.h					\
Source.h					\
StationSim.h					\
Trajectory.h					\
MDL.h						\
WH_AddSignals.h					\
WH_CreateSource.h				\
WH_Modulate.h					\
WH_PhaseShift.h					\
WH_ReadSignal.h					\
WH_BandSep.h					\
GnuPlotInterface.h				\
WH_BeamFormer.h					\
WH_STA.h					\
WH_WeightDetermination.h			\
WH_Projection.h					\
WH_SubBandSel.h					\
GnuPlotInterface.h				\
QMinterface.h					\
PASTd.h						\
DH_CubeC.h                                      \
WH_DataGenerator.h                              \
WH_Station.h					\
WH_LopesDataReader.h

lib_LTLIBRARIES 	= libstationsim.la

libstationsim_la_SOURCES 	= $(DOCHDRS)	\
ArrayConfig.cc					\
DH_SampleC.cc					\
DH_SampleR.cc					\
DH_SubBandSel.cc				\
DataGenConfig.cc				\
FFTW.cc						\
FilterBank.cc					\
Modulate.cc					\
PhaseShift.cc					\
Signal.cc					\
Source.cc					\
StationSim.cc					\
Trajectory.cc					\
MDL.cc						\
WH_AddSignals.cc				\
WH_CreateSource.cc				\
WH_Modulate.cc					\
WH_PhaseShift.cc				\
WH_ReadSignal.cc				\
WH_BandSep.cc					\
GnuPlotInterface.cc				\
WH_BeamFormer.cc				\
WH_STA.cc					\
WH_WeightDetermination.cc			\
WH_Projection.cc				\
WH_SubBandSel.cc				\
GnuPlotInterface.cc				\
QMinterface.cc					\
PASTd.cc					\
DH_CubeC.cc                                     \
WH_DataGenerator.cc                             \
WH_Station.cc					\
WH_LopesDataReader.cc

bin_PROGRAMS		= Simulate
Simulate_SOURCES 	= Simulate.cc Simulate.in

Simulate_LDADD		= libstationsim.la
Simulate_DEPENDENCIES	= libstationsim.la $(LOFAR_DEPEND)

#EXTRA_DIST		=			\
#Beam.m						\
#spectralfft.m					\
#Simulate.out

include $(lofar_sharedir)/Makefile.common


