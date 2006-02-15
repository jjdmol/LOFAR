# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

INSTHDRS        = Connector.h \
	 InputThread.h \
	 LockedRange.h \
	 LockedRange.tcc \
	 BeamletBuffer.h \
	 AH_InputSection.h \
	 WH_SBCollect.h \
	 WH_RSPInput.h

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
