INSTHDRS         	=\
WH_Distribute.h		\
WH_PPF.h		\
WH_Correlator.h		\
WH_Concentrator.h	\
AH_BGLProcessing.h	

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_BGLProcessing

TFC_BGLProcessing_SOURCES	= $(DOCHDRS) \
WH_Distribute.cc		\
WH_PPF.cc			\
FIR.s				\
WH_Correlator.cc		\
WH_Concentrator.cc		
# TFC_BGLProcessing_main.cc 	\
# AH_BGLProcessing.cc		

configfilesdir=$(bindir)
configfiles_DATA	=	\
TFC_BGLProcessing.machinefile	\
TFC_BGLProcessing.log_prop	

include $(lofar_sharedir)/Makefile.common
