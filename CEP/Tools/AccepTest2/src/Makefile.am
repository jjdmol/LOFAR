bin_PROGRAMS	= testWHs

INSTHDRS	=				\
DH_CMatrix.h					\
WH_Random.h					\
WH_Transpose.h					\
WH_Heat.h					\
WH_Split.h					\
WH_Join.h					\
WH_Dump.h					

DOCHDRS 	= $(INSTHDRS)

NONPROGSRCS =					\
DH_CMatrix.cc					\
WH_Random.cc					\
WH_Transpose.cc					\
WH_Heat.cc					\
WH_Split.cc					\
WH_Join.cc					\
WH_Dump.cc					

testWHs_SOURCES		= $(DOCHDRS)		\
$(NONPROGSRCS)					\
Simulate.cc					\
AH_testWHs.h					\
AH_testWHs.cc

testWHs_DEPENDENCIES = $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
