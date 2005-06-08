INSTHDRS         	=\
DH_CorrCube.h		\
DH_Vis.h		\
DH_SubBand.h		\
DH_RSP.h                \
WH_Correlator.h		\
WH_SubBand.h		\
CyclicBuffer.h          \
BufferController.h      \
RSPTimeStamp.h          \
WH_RSPInput.h           \
SB_Stub.h               


NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_Correlator TFC_Input TFC_Store

TFC_Correlator_SOURCES 	= $(DOCHDRS)\
TFlopCorrelator_main.cc \
DH_CorrCube.cc		\
DH_Vis.cc		\
DH_SubBand.cc		\
DH_RSP.cc               \
WH_RSPInput.cc          \
WH_Correlator.cc	\
WH_SubBand.cc		\
SB_Stub.cc              \
RSPTimeStamp.cc         

TFC_Input_SOURCES	= $(DOCHDRS)\
TFlopCorrelator_main.cc \
DH_SubBand.cc		\
SB_Stub.cc

TFC_Store_SOURCES 	= $(DOCHDRS)\
TFlopCorrelator_main.cc \
DH_Vis.cc		\
Corr_Stub.cc

include $(lofar_sharedir)/Makefile.common
