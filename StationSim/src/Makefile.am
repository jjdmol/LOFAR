DOCHDRS		 	= 			\
DH_SampleR.h					\
DH_SampleC.h					\
WH_BandSep.h					\
DH_SubBandSel.h					\
WH_SubBandSel.h					\
WH_Selector.h					\
DH_Weight.h					\
WH_BeamFormer.h					\
WH_AWE.h					\
WH_Detection.h					\
WH_Calibration.h				\
WH_Cancel.h					

lib_LTLIBRARIES 	= libstationsim.la

libstationsim_la_SOURCES 	= $(DOCHDRS)	\
DH_SampleR.cc					\
DH_SampleC.cc					\
WH_BandSep.cc					\
DH_SubBandSel.cc				\
WH_SubBandSel.cc				\
WH_Selector.cc					\
DH_Weight.cc					\
WH_BeamFormer.cc				\
WH_AWE.cc					\
StationSim.cc					\
WH_Detection.cc					\
WH_Calibration.cc				\
WH_Cancel.cc					

bin_PROGRAMS		= Simulate
Simulate_SOURCES 	= Simulate.cc Simulate.in

Simulate_LDADD		= libstationsim.la
Simulate_DEPENDENCIES	= libstationsim.la $(LOFAR_DEPEND)

#EXTRA_DIST		=			\
#Beam.m						\
#spectralfft.m					\
#Simulate.out

include $(lofar_sharedir)/Makefile.common
