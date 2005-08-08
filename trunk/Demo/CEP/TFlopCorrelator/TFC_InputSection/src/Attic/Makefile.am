INSTHDRS         	=\
WH_RSPInput.h           \
WH_SBCollect.h		\
WH_Transpose.h		\
AH_InputSection.h	\
CyclicBuffer.h          \
BufferController.h   


NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_InputSection

TFC_InputSection_SOURCES	= $(DOCHDRS) \
TFC_InputSection_main.cc	\
WH_RSPInput.cc           	\
WH_SBCollect.cc			\
WH_Transpose.cc			\
AH_InputSection.cc		\
BufferController.cc	

include $(lofar_sharedir)/Makefile.common
