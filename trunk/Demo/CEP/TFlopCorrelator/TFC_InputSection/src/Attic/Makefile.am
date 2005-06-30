INSTHDRS         	=\
WH_RSPInput.h           \
WH_Transpose.h		\
WH_SyncControl.h	\
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
WH_Transpose.cc			\
WH_SyncControl.cc		\
AH_InputSection.cc			

include $(lofar_sharedir)/Makefile.common
