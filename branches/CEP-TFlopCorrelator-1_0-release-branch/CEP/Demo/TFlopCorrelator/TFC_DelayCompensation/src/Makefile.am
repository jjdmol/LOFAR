INSTHDRS         	=\
AH_DelayCompensation.h		\
WH_DelayControl.h


NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_DelayCompensation

TFC_DelayCompensation_SOURCES 	= $(DOCHDRS) \
TFC_DelayCompensation_main.cc 	\
AH_DelayCompensation.cc		\
WH_DelayControl.cc

configfilesdir=$(bindir)
configfiles_DATA		=	\
TFC_DelayCompensation.machinefile		\
TFC_DelayCompensation.log_prop		

include $(lofar_sharedir)/Makefile.common
