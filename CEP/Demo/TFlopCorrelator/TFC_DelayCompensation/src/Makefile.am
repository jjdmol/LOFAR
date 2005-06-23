INSTHDRS         	=\
AH_DelayCompensation.h		


NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_DelayCompensation

TFC_DelayCompensation_SOURCES 	= $(DOCHDRS) \
TFC_DelayCompensation_main.cc 	\
AH_DelayCompensation.cc		

include $(lofar_sharedir)/Makefile.common
