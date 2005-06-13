INSTHDRS         	=\
DH_Sync.h		\
DH_RSP.h                \
DH_SubBand.h		\
DH_CorrCube.h		\
DH_Vis.h		\
WH_RSPInput.h           \
WH_Transpose.h		\
WH_SubBand.h		\
WH_Correlator.h		\
WH_SyncControl.h	\
AH_InputSection.h	\
AH_BGLProcessing.h	\
CyclicBuffer.h          \
BufferController.h      \
RSPTimeStamp.h          \
Stub_SB.h               \
Stub_Corr.h		\
Stub_RSP.h
#WH_RSP.h 	
#WH_Collect.h
#WH_Store.h
#AH_Storage.h

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_InputSection TFC_BGLProcessing TFC_Storage

TFC_InputSection_SOURCES	= $(DOCHDRS) \
TFC_InputSection_main.cc	\
DH_Sync.cc			\
DH_RSP.cc                	\
DH_SubBand.cc			\
WH_RSPInput.cc           	\
WH_Transpose.cc			\
WH_SyncControl.cc		\
RSPTimeStamp.cc          	\
Stub_SB.cc               	\
Stub_RSP.cc			\
AH_InputSection.cc		
#WH_RSP.cc 			

TFC_BGLProcessing_SOURCES 	= $(DOCHDRS) \
TFC_BGLProcessing_main.cc 	\
DH_SubBand.cc			\
DH_CorrCube.cc			\
DH_Vis.cc			\
WH_SubBand.cc			\
WH_Correlator.cc		\
AH_BGLProcessing.cc		\
Stub_SB.cc             		\
Stub_Corr.cc
#WH_Collect.cc


TFC_Storage_SOURCES 	= $(DOCHDRS) \
TFC_Storage_main.cc 	\
DH_Vis.cc		\
Stub_Corr.cc		\
AH_Storage.cc
#WH_Store.h

include $(lofar_sharedir)/Makefile.common
