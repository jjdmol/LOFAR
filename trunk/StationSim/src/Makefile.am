DOCHDRS		 	= 			\
DH_RCU.h					\
WH_RCU.h					\
WH_RCUAdd.h					\
WH_Merge.h					\
WH_RCUAll.h					\
DH_SampleR.h					\
DH_SampleC.h					\
WH_BandSep.h					\
DH_SubBandSel.h					\
WH_SubBandSel.h					\
WH_Selector.h					\
DH_Weight.h					\
DH_TargetTrack.h				\
WH_TargetTrack.h				\
DH_RFITrack.h					\
WH_RFITrack.h					\
WH_BeamFormer.h					\
WH_AWE.h					\
WH_Analysis.h					\
WH_Detection.h					\
WH_Calibration.h				\
WH_Cancel.h					\
WH_VerifyRFI.h

lib_LTLIBRARIES 	= libstationsim.la

libstationsim_la_SOURCES 	= $(DOCHDRS)	\
DH_RCU.cc					\
WH_RCU.cc					\
WH_RCUAdd.cc					\
WH_Merge.cc					\
WH_RCUAll.cc					\
DH_SampleR.cc					\
DH_SampleC.cc					\
WH_BandSep.cc					\
DH_SubBandSel.cc				\
WH_SubBandSel.cc				\
WH_Selector.cc					\
DH_Weight.cc					\
DH_TargetTrack.cc				\
WH_TargetTrack.cc				\
DH_RFITrack.cc					\
WH_RFITrack.cc					\
WH_BeamFormer.cc				\
WH_AWE.cc					\
WH_Analysis.cc					\
StationSim.cc					\
WH_Detection.cc					\
WH_Calibration.cc				\
WH_Cancel.cc					\
WH_VerifyRFI.cc

bin_PROGRAMS		= Simulate
Simulate_SOURCES 	= Simulate.cc Simulate.in

Simulate_LDADD		= libstationsim.la
Simulate_DEPENDENCIES	= libstationsim.la $(LOFAR_DEPEND)

#EXTRA_DIST		=			\
#Beam.m						\
#spectralfft.m					\
#Simulate.out

include $(lofar_sharedir)/Makefile.common
