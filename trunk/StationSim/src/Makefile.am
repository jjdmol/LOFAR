DOCHDRS		 	= 			\
DH_RCU.h					\
WH_RCU.h					\
WH_RCUAdd.h					\
WH_Merge.h					\
DH_Sample.h					\
WH_Filter.h					\
DH_SubBandSel.h					\
WH_SubBandSel.h					\
WH_Selector.h					\
DH_Weight.h					\
DH_TargetTrack.h				\
WH_TargetTrack.h				\
DH_RFITrack.h					\
WH_RFITrack.h					\
WH_BeamFormer.h					\
WH_AWE.h					

lib_LTLIBRARIES 	= libstationsim.la

libstationsim_la_SOURCES 	= $(DOCHDRS)	\
DH_RCU.cc					\
WH_RCU.cc					\
WH_RCUAdd.cc					\
WH_Merge.cc					\
DH_Sample.cc					\
WH_Filter.cc					\
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
StationSim.cc

bin_PROGRAMS		= Simulate
Simulate_SOURCES 	= Simulate.cc Simulate.in

Simulate_LDADD		= ../src/libstationsim.la
Simulate_DEPENDENCIES	= ../src/libstationsim.la $(LOFAR_DEPEND)

#EXTRA_DIST		=			\
#Beam.m						\
#spectralfft.m					\
#Simulate.out

include $(lofar_sharedir)/Makefile.common
