INSTHDRS         	=\
WH_PPF.h		\
WH_Correlator.h		\
AH_BGLProcessing.h	

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_BGLProcessing

CCAS			= gcc
CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) -B/bgl/BlueLight/ppcfloor/blrts-gnu/powerpc-bgl-blrts-gnu/bin/


TFC_BGLProcessing_SOURCES	= $(DOCHDRS) \
TFC_BGLProcessing_main.cc 	\
WH_PPF.cc			\
WH_Correlator.cc		\
AH_BGLProcessing.cc		\
FFT.S				\
FIR.S
#WH_Collect.cc

include $(lofar_sharedir)/Makefile.common
