INSTHDRS         	=\
AH_Storage.h		\
WH_Storage.h		\
MSWriter.h		\
MSWriterImpl.h

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_Storage

TFC_Storage_SOURCES 	= $(DOCHDRS) \
TFC_Storage_main.cc 	\
AH_Storage.cc		\
WH_Storage.cc		\
MSWriter.cc		\
MSWriterImpl.cc

configfilesdir=$(bindir)
configfiles_DATA		=	\
TFC_Storage.machinefile			\
TFC_Storage.log_prop			\
typeStruct_TTeraFlopCorrelator.dpl	\
PropertyInterface.conf

include $(top_srcdir)/Makefile.common
