bin_PROGRAMS = RTCorrelator RTFrontEnd

DOCHDRS			=				\
Config.h						\
RTCorrelator.h						\
RTFrontEnd.h						\
DH_Vis.h						\
DH_CorrCube.h						\
WH_Correlator.h						\
WH_Random.h						\
WH_Dump.h						\
DH_Empty.h						

RTCorrelator_SOURCES	=	$(DOCHDRS)		\
Config.cc						\
RTCorrelator.cc						\
DH_Vis.cc						\
DH_CorrCube.cc						\
WH_Correlator.cc					\
WH_Random.cc						\
WH_Dump.cc						\
DH_Empty.cc						

RTFrontEnd_SOURCES 	=	$(DOCHDRS)		\
Config.cc						\
RTFrontEnd.cc						\
DH_Vis.cc						\
DH_CorrCube.cc						\
DH_Empty.cc						\
WH_Random.cc						\
WH_Correlator.cc					\
WH_Dump.cc						


RTCorrelator_DEPENDENCIES = $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common