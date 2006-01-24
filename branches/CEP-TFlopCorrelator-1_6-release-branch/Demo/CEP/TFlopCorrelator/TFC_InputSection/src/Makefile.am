INSTHDRS		= \
LockedRange.h 		\
LockedRange.tcc		\
BeamletBuffer.h 	\
WH_RSPInput.h           \
WH_SBCollect.h		\
AH_InputSection.h	\
BufferController.h      \
CyclicCounter.h		\
ReaderThread.h


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
AH_InputSection.cc		\
BufferController.cc	        \
BeamletBuffer.cc		\
CyclicCounter.cc		\
ReaderThread.cc

configfilesdir=$(bindir)
configfiles_DATA		=	\
TFC_InputSection.machinefile		\
TFC_InputSection.log_prop		

include $(top_srcdir)/Makefile.common
