bin_PROGRAMS	=  BlueGeneCorrelator BlueGeneFrontEnd

DOCHDRS 	=				\
DH_CorrCube.h					\
DH_Vis.h					\
DH_Empty.h					\
WH_Correlate.h					\
WH_Dump.h					\
WH_Random.h					\
BlueGeneCorrelator.h				\
BlueGeneFrontEnd.h				\
defintions.h

BlueGeneCorrelator_SOURCES = $(DOCHDRS)		\
DH_Empty.cc					\
DH_CorrCube.cc					\
DH_Vis.cc					\
WH_Random.cc					\
WH_Correlate.cc					\
WH_Dump.cc					\
BlueGeneSimulateBG.cc				\
BlueGeneCorrelator.cc

BlueGeneFrontEnd_SOURCES = $(DOCHDRS)		\
DH_Empty.cc					\
DH_CorrCube.cc					\
DH_Vis.cc					\
WH_Random.cc					\
WH_Correlate.cc					\
WH_Dump.cc					\
BlueGeneFrontEnd.cc				\
BlueGeneSimulateFE.cc		

BlueGeneFrontEnd_DEPENDENCIES = $(LOFAR_DEPEND)
BlueGeneCorrelator_DEPENDENCIES = $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
