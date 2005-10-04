INSTHDRS         	=\
WH_FIR.h		\
WH_FFT.h		\
WH_Correlator.h		\
AH_BGLProcessing.h	

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_BGLProcessing

TFC_BGLProcessing_SOURCES	= $(DOCHDRS) \
TFC_BGLProcessing_main.cc 	\
WH_FIR.cc			\
WH_FFT.cc			\
WH_Correlator.cc		\
AH_BGLProcessing.cc		
#WH_Collect.cc

configfilesdir=$(bindir)
configfiles_DATA	=	\
TFC_BGLProcessing.machinefile	\
TFC_BGLProcessing.log_prop	

include $(top_srcdir)/Makefile.common
