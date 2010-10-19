# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

INSTHDRS =      \
DH_DB.h         \
TH_DB.h         \
TH_Postgresql.h

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
