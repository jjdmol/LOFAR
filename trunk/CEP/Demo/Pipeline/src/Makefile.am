bin_PROGRAMS	= Pipeline

DOCHDRS 	=				\
DH_2DMatrix					\
DH_Correlations.h				\
Pipeline.h					\
WH_FillTFMatrix.h				\
WH_Delay.h					\
WH_Transpose.h					\
WH_Correlate.h					\
WH_PreCorrect.h					\
WH_Dump.h

Pipeline_SOURCES  	= $(DOCHDRS)		\
DH_2DMatrix.cc					\
DH_Correlations.cc				\
Pipeline.cc					\
Simulate.cc					\
WH_FillTFMatrix.cc				\
WH_Delay.cc					\
WH_Transpose.cc					\
WH_PreCorrect.cc				\
WH_Correlate.cc					\
WH_Dump.cc				

Pipeline_DEPENDENCIES = $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
