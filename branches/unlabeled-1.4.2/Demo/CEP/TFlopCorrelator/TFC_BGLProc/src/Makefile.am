INSTHDRS         	=\
AH_BGLProcessing.h	\
WH_PPF.h		\
WH_Correlator.h		\
WH_Distribute.h		\
WH_Concentrator.h
# WH_BGL_Processing.h	

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_BGLProcessing

CCAS			= gcc
CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) -B/bgl/BlueLight/ppcfloor/blrts-gnu/powerpc-bgl-blrts-gnu/bin/

TFC_BGLProcessing_SOURCES	= $(DOCHDRS) \
Correlator.S			\
FIR.S				\
TFC_BGLProcessing_main.cc 	\
AH_BGLProcessing.cc		\
WH_PPF.cc			\
WH_Correlator.cc		\
WH_Distribute.cc		\
WH_Concentrator.cc		
# WH_BGL_Processing.cc		

include $(lofar_sharedir)/Makefile.common
