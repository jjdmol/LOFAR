# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

INSTHDRS =    	\
AH_Storage.h	\
WH_SubbandWriter.h   	\
MSWriter.h     	\
MSWriterImpl.h

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
