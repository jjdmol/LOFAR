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

include $(lofar_sharedir)/Makefile.common
